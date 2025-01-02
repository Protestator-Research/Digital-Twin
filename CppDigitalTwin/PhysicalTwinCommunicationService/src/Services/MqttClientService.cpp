//
// Created by Moritz Herzog on 06.08.24.
//

#include "MqttClientService.h"

#include "../entities/DigitalTwinEntity.h"

#include <iostream>
#include <async_mqtt/predefined_layer/ws.hpp>
#include <thread>

#include <boost/asio.hpp>
#include <boost/asio/async_result.hpp>

//using client_t = async_mqtt::client<async_mqtt::protocol_version::v5,async_mqtt::protocol::mqtt>;

namespace PHYSICAL_TWIN_COMMUNICATION {


    MqttClientService::MqttClientService(std::string server, std::string port) :
            Client(client_t::create(ioContext.get_executor()))
    {
        Server = server;
        Port = port;
    }

    MqttClientService::~MqttClientService() {

    }

    void MqttClientService::sendValueToServer(std::string topic, std::string content) {
            Client->async_publish(
                    *Client->acquire_unique_packet_id(), // sync version only works thread safe context
                    topic,
                    content,
                    async_mqtt::qos::at_least_once,
                    [this](auto&&... args) {
                        handlePublishResponse(
                                std::forward<std::remove_reference_t<decltype(args)>>(args)...
                        );
                    }
            );
    }

    void MqttClientService::addCallbackFunction(const std::string& topic, std::function<void(std::string)> callbackFunction) {
        CallbackFuctionsPerTopic[topic] = callbackFunction;
        std::vector<async_mqtt::topic_subopts> sub_entry;

        for(auto element : CallbackFuctionsPerTopic) {
            sub_entry.push_back({element.first, async_mqtt::qos::at_least_once});
        }

        if(ClientStarted) {
            Client->async_subscribe(
                    *Client->acquire_unique_packet_id(),
                    async_mqtt::force_move(sub_entry),
                    [this](auto&&... args) {
                        handleSubscribeResponse(
                                std::forward<std::remove_reference_t<decltype(args)>>(args)...
                        );
                    }
                    );
//        uint16_t packetId = Client->subscribe(topic, MQTT_NS::qos::at_least_once);
//        PackedIdToTopicMapping[packetId] = topic;
        }
    }

    void MqttClientService::connectClientStartCommunication() {
        std::cout << "Starting underlying handshake-" << std::endl;
        auto next_layer = &Client->next_layer();
        async_mqtt::async_underlying_handshake(
                *next_layer,
                Server,
                Port,
                [this](auto&&... args) {
                    handleUnderlyingHandshake(
                            std::forward<std::remove_reference_t<decltype(args)>>(args)...
                    );
                }
        );
        ioContext.run();
    }

    void MqttClientService::handleUnderlyingHandshake(async_mqtt::error_code errorCode) {
        std::cout << "underlying_handshake:" << errorCode.message() << std::endl;
        if(errorCode) return;
        Client->async_start(true,
                           std::uint16_t(0),
                           "",
                           std::nullopt,
                           "", //Username
                           "", //Password
                           [this](auto&&... args) {
                               handleStartResponse(std::forward<decltype(args)>(args)...);
                           }
                           );
    }

    void MqttClientService::handleStartResponse(async_mqtt::error_code ec,
                                                std::optional<client_t::connack_packet> connack_opt) {
        std::cout << "start:" << ec.message() << std::endl;
        if (ec) return;
        if (connack_opt) {
            std::cout << *connack_opt << std::endl;
            Client->async_publish(
                    *Client->acquire_unique_packet_id(),
                    "openDigitalTwin",
                    DigitalTwinEntity().serialize(),
                    async_mqtt::qos::at_least_once,
                    [this](auto&&... args) {
                        handlePublishResponse(
                                std::forward<std::remove_reference_t<decltype(args)>>(args)...
                        );
                    }
            );
        }
    }

    void MqttClientService::handlePublishResponse(async_mqtt::error_code ec, client_t::pubres_type pubres) {
        std::cout << "publish:" << ec.message() << std::endl;
        if (ec) return;
        if (pubres.puback_opt) {
            std::cout << *pubres.puback_opt << std::endl;
        }
        if (pubres.pubrec_opt) {
            std::cout << *pubres.pubrec_opt << std::endl;
        }
        if (pubres.pubcomp_opt) {
            std::cout << *pubres.pubcomp_opt << std::endl;
            Client->async_disconnect(boost::asio::detached);
        }
    }

    void MqttClientService::handleSubscribeResponse(async_mqtt::error_code ec, std::optional<client_t::suback_packet> suback_opt) {
        std::cout << "subscribe:" << ec.message() << std::endl;
        if (ec) {
            //reconnect();
            return;
        }
        if (suback_opt) {
            std::cout << *suback_opt << std::endl;
        }
        Client->async_recv(
                [this](auto&&... args) {
                    handleReceive(
                            std::forward<std::remove_reference_t<decltype(args)>>(args)...
                    );
                }
        );
    }

    void MqttClientService::handleReceive(async_mqtt::error_code ec, async_mqtt::packet_variant pv) {
        std::cout << "recv:" << ec.message() << std::endl;
        if (ec) {
//            reconnect();
            return;
        }
        BOOST_ASSERT(pv);
        std::cout << pv << std::endl;
        // next receive
        Client->async_recv(
                [this](auto&&... args) {
                    handleReceive(
                            std::forward<std::remove_reference_t<decltype(args)>>(args)...
                    );
                }
        );
    }


}