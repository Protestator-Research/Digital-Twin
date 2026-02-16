//
// Created by Moritz Herzog on 06.08.24.
//

#include "MqttClientService.h"

#include "../MQTT/entities/DigitalTwinEntity.h"
#include "../MQTT/Topics.h"

#include <iostream>
#include <async_mqtt/all.hpp>
#include <thread>
#include <optional>

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/use_awaitable.hpp>

namespace PHYSICAL_TWIN_COMMUNICATION {
    MqttClientService::MqttClientService(std::string server, std::string port, std::string clientId) : KeepAlive(60),
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
        WorkerThread = std::thread([this] {
            boost::asio::co_spawn(Strand, [this]() -> boost::asio::awaitable<void> { co_await run(); }, boost::asio::detached);
            IoContext.run();
        });
    }

    void MqttClientService::stop() {
        if (!ClientStarted.exchange(false)) return;
        boost::asio::post(Strand, [this] {
            boost::asio::co_spawn(Strand, [this]() -> boost::asio::awaitable<void> {
                try { co_await Client.async_close(boost::asio::use_awaitable); } catch (...) {}
                IoContext.stop();
                co_return;
            }, boost::asio::detached);
        });
        if (WorkerThread.joinable()) WorkerThread.join();
    }

    void MqttClientService::publish(std::string topic, std::string payload, async_mqtt::qos qos) {
        boost::asio::post(Strand, [this, topic=std::move(topic), payload=std::move(payload), qos] {
        if (!Connected) return;
        boost::asio::co_spawn(Strand, [this, topic, payload, qos]() -> boost::asio::awaitable<void> {
            co_await Client.async_publish(async_mqtt::v5::publish_packet{topic, payload, qos}, boost::asio::use_awaitable);
            co_return;
            }, boost::asio::detached);
        });
    }

    std::future<std::string> MqttClientService::request(std::string topic, std::string payload, std::string respTopic) {
        std::promise<std::string> prom;
        auto fut = prom.get_future();

        boost::asio::post(Strand, [this,
                          req_topic=std::move(topic),
                          req_payload=std::move(payload),
                          resp_topic=std::move(respTopic),
                          prom=std::move(prom)]() mutable {
            if (!Connected) {
                prom.set_exception(std::make_exception_ptr(std::runtime_error("Not connected")));
                return;
            }

            auto corr = makeCorrelationData();
            std::string corr_key(reinterpret_cast<char const*>(corr.data()), corr.size());
            Pending[corr_key] = std::move(prom);

            // subscribe response topic einmalig (hier minimalistisch)
            if (!HasResponseSubscription) {
                HasResponseSubscription = true;
                boost::asio::co_spawn(Strand, [this, resp_topic]() -> boost::asio::awaitable<void> {
                    auto pid = co_await Client.async_acquire_unique_packet_id_wait_until(boost::asio::use_awaitable);
                    if (!pid) co_return;
                    std::vector<async_mqtt::topic_subopts> entries{{resp_topic, async_mqtt::qos::at_most_once}};
                    co_await Client.async_subscribe(async_mqtt::v5::subscribe_packet{pid, async_mqtt::force_move(entries)},
                                                     boost::asio::use_awaitable_t<boost::asio::any_io_executor>{});
                    co_return;
                }, boost::asio::detached);
            }

            // publish request mit ResponseTopic + CorrelationData
            boost::asio::co_spawn(Strand, [this, req_topic, req_payload, resp_topic, corr_key]() -> boost::asio::awaitable<void> {
                std::vector<async_mqtt::property_variant> props;
                props.emplace_back(async_mqtt::property::response_topic{resp_topic});
                props.emplace_back(async_mqtt::property::correlation_data{corr_key});
                const auto package = async_mqtt::v5::publish_packet{
                        req_topic, req_payload, async_mqtt::qos::at_most_once, async_mqtt::properties{async_mqtt::force_move(props)}
                    };
                co_await Client.async_publish(
                    std::move(package),
                    boost::asio::use_awaitable_t<boost::asio::any_io_executor>{}
                );
                co_return;
            }, boost::asio::detached);
        });

        return fut;
    }

    boost::asio::awaitable<void> MqttClientService::run() {
        // 1) TCP handshake :contentReference[oaicite:5]{index=5}
        co_await Client.async_underlying_handshake(Server, Port, boost::asio::use_awaitable);

        // 2) MQTT CONNECT + start receive loop :contentReference[oaicite:6]{index=6}
        auto connack_opt = co_await Client.async_start(
            async_mqtt::v5::connect_packet{
                true,
                (uint16_t)KeepAlive.count(),
                ClientId
            },
            boost::asio::use_awaitable
        );
        (void)connack_opt;
        Connected = true;

        // 3) Receive dispatcher (holt PUBLISH/DISCONNECT/AUTH aus interner Queue) :contentReference[oaicite:7]{index=7}
        while (ClientStarted) {
            auto pv = co_await Client.async_recv(boost::asio::use_awaitable);

            pv->visit(async_mqtt::overload{
                [&](async_mqtt::v5::publish_packet& p) {
                    const std::string topic = p.topic();
                    const std::string payload = p.payload();

                    // Request/Response: correlation_data suchen
                    if (auto corr_key = extractCorrelationKey(p)) {
                        auto it = Pending.find(*corr_key);
                        if (it != Pending.end()) {
                            it->second.set_value(payload);
                            Pending.erase(it);
                            return;
                        }
                    }

                    // normale subscriptions (hier nur exact match)
                    if (auto it = Subscriptions.find(topic); it != Subscriptions.end()) {
                        it->second(topic, payload);
                    }
                },
                [&](async_mqtt::v5::disconnect_packet&) {
                    Connected = false;
                },
                [&](auto&) {
                    // ignore
                }
            });
        }

        co_return;
    }

    std::vector<uint8_t> MqttClientService::makeCorrelationData() {
        static std::uint64_t counter = 0;
        auto v = ++counter;

        std::vector<std::uint8_t> data(sizeof(v));
        std::memcpy(data.data(), &v, sizeof(v));
        return data;
    }

    std::optional<std::string> MqttClientService::extractCorrelationKey(async_mqtt::v5::publish_packet const &packet) {
        // publish_packet besitzt properties; correlation_data ist ein MQTT v5 property
        // (Konkrete Property-Container-Typen sind in async_mqtt als property_variant modelliert)
        for (auto const& prop : packet.props()) {
            if (auto cd = prop.get_if<async_mqtt::property::correlation_data>()) {
                auto const& b = cd->val();
                return std::string(reinterpret_cast<const char*>(b.data()), b.size());
            }
        }
        return std::nullopt;
    }
}
