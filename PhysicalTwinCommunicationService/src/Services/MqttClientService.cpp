//
// Created by Moritz Herzog on 06.08.24.
//

#include "MqttClientService.h"

#include "../MQTT/entities/DigitalTwinEntity.h"
#include "../MQTT/Topics.h"

#include <iostream>
#include <exception>
#include <async_mqtt/all.hpp>
#include <thread>
#include <optional>

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/use_awaitable.hpp>

namespace DigitalTwin::Communication {
    MqttClientService::MqttClientService(std::string server, std::string port, std::string clientId) : KeepAlive(60),
        Strand(IoContext.get_executor()),
        Client(Strand),
        ClientStarted(false),
        Connected(false) {
        Server = server;
        Port = port;
        ClientId = clientId;
    }

    MqttClientService::~MqttClientService() {
        stop();
    }

    void MqttClientService::start() {
        if (ClientStarted.exchange(true)) return;
        WorkGuard.emplace(boost::asio::make_work_guard(IoContext));
        boost::asio::co_spawn(Strand, [this]() -> boost::asio::awaitable<void> { co_await run(); }, [](std::exception_ptr ep) {
            if (ep) {
                try {
                    std::rethrow_exception(ep);
                } catch (std::exception const& e) {
                    std::cerr << "MQTT client receive loop terminated: " << e.what() << std::endl;
                } catch (...) {
                    std::cerr << "MQTT client receive loop terminated: unknown exception" << std::endl;
                }
            }
        });
        WorkerThread = std::thread([this] {
            IoContext.run();
        });
    }

    void MqttClientService::stop() {
        if (!ClientStarted.exchange(false)) return;
        boost::asio::post(Strand, [this] {
            boost::asio::co_spawn(Strand, [this]() -> boost::asio::awaitable<void> {
                try { co_await Client.async_close(boost::asio::use_awaitable); } catch (...) {}
                Connected = false;
                WorkGuard.reset();
                IoContext.stop();
                co_return;
            }, boost::asio::detached);
        });
        if (WorkerThread.joinable() && WorkerThread.get_id() != std::this_thread::get_id())
            WorkerThread.join();
    }

    void MqttClientService::publish(std::string topic, std::string payload) {
        publish(topic, payload, async_mqtt::qos::at_most_once);
    }

    void MqttClientService::publish(std::string topic, std::string payload, async_mqtt::qos qos) {
        if (topic.empty() || payload.empty())
            return;

        boost::asio::post(Strand, [this, topic=std::move(topic), payload=std::move(payload), qos] {
        if (!ClientStarted) return;
        boost::asio::co_spawn(Strand, [this, topic, payload, qos]() -> boost::asio::awaitable<void> {
            co_await Client.async_publish(async_mqtt::v3_1_1::publish_packet{topic, payload, qos}, boost::asio::use_awaitable);
            co_return;
            }, boost::asio::detached);
        });
    }

    void MqttClientService::subscribe(std::string topic, std::function<void(std::string topic, std::string payload)> callback) {
        boost::asio::post(Strand, [this, topic = std::move(topic), callback = std::move(callback)]() mutable {

            Callbacks[topic] = std::move(callback);
            Subscriptions.emplace(std::make_pair(topic, callback));

            if (!Connected) return;

            boost::asio::co_spawn(
                Strand,
                [this, topic]() -> boost::asio::awaitable<void, boost::asio::any_io_executor> {

                    auto pid =
                        co_await Client.async_acquire_unique_packet_id_wait_until(
                            boost::asio::use_awaitable_t<boost::asio::any_io_executor>{});

                    if (!pid) co_return;

                    std::vector<async_mqtt::topic_subopts> entries{
                        {topic, async_mqtt::qos::at_most_once}
                    };

                    co_await Client.async_subscribe(
                        async_mqtt::v3_1_1::subscribe_packet{
                            pid,
                            async_mqtt::force_move(entries)
                        },
                        boost::asio::use_awaitable_t<boost::asio::any_io_executor>{}
                    );

                    co_return;
                },
                boost::asio::detached
            );
        }
    );
    }

    boost::asio::awaitable<void> MqttClientService::run() {
        co_await Client.async_underlying_handshake(Server, Port, boost::asio::use_awaitable);

        auto connack_opt = co_await Client.async_start(
            async_mqtt::v3_1_1::connect_packet{
                true,
                (uint16_t)KeepAlive.count(),
                ClientId
            },
            boost::asio::use_awaitable
        );
        (void)connack_opt;
        Connected = true;

        // Subscriptions registered before the connection was established have to be sent now.
        // No suspension point between Connected = true and the snapshot, so subscribe() cannot subscribe twice.
        std::vector<std::string> pendingTopics;
        for (auto const& entry : Callbacks)
            pendingTopics.push_back(entry.first);
        // The SUBACK is delivered via async_recv below, so the subscriptions run as separate coroutines.
        for (auto const& pendingTopic : pendingTopics) {
            boost::asio::co_spawn(Strand, [this, pendingTopic]() -> boost::asio::awaitable<void> {
                auto pid = co_await Client.async_acquire_unique_packet_id_wait_until(boost::asio::use_awaitable);
                if (!pid) co_return;
                std::vector<async_mqtt::topic_subopts> entries{{pendingTopic, async_mqtt::qos::at_most_once}};
                co_await Client.async_subscribe(async_mqtt::v3_1_1::subscribe_packet{pid, async_mqtt::force_move(entries)},
                                                boost::asio::use_awaitable);
                co_return;
            }, boost::asio::detached);
        }

        // 3) Receive dispatcher: takes the PUBLISH packets from the internal queue. MQTT 3.1.1 has no server DISCONNECT,
        // a lost connection shows up as an error of async_recv (use_awaitable turns it into an exception).
        while (ClientStarted) {
            std::optional<async_mqtt::packet_variant> pv;
            try {
                pv = co_await Client.async_recv(boost::asio::use_awaitable);
            } catch (std::exception const& e) {
                if (ClientStarted)
                    std::cerr << "MQTT connection lost: " << e.what() << std::endl;
                Connected = false;
                break;
            }
            if (!pv) {
                Connected = false;
                break;
            }

            pv->visit(async_mqtt::overload{
                [&](async_mqtt::v3_1_1::publish_packet& p) {
                    const std::string topic = p.topic();
                    const std::string payload = p.payload();

                    // normale subscriptions (hier nur exact match)
                    if (auto it = Callbacks.find(topic); it != Callbacks.end()) {
                        try {
                            it->second(topic, payload);
                        } catch (std::exception const& e) {
                            std::cerr << "Exception in subscription callback for topic '" << topic << "': " << e.what() << std::endl;
                        } catch (...) {
                            std::cerr << "Unknown exception in subscription callback for topic '" << topic << "'" << std::endl;
                        }
                    }
                },
                [&](auto&) {
                    // ignore
                }
            });
        }

        co_return;
    }
}
