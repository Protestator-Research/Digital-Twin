//
// Created by Moritz Herzog on 06.08.24.
//

#include "MqttClientService.h"

#include "../entities/DigitalTwinEntity.h"

#include <iostream>
#include <thread>

using client_t = async_mqtt::client<async_mqtt::protocol_version::v5,async_mqtt::protocol::mqtt>;

namespace PHYSICAL_TWIN_COMMUNICATION {


    MqttClientService::MqttClientService(std::string server, std::string port) :
            Client(client_t::create(ioContext.get_executor()))
    {
        Server = server;
        Port = port;
    }

    MqttClientService::~MqttClientService() {

    }

    void MqttClientService::sendValueToServer(std::string , std::string ) {
//        Client->publish(Client->acquire_unique_packet_id(), topic, value);
    }

    void MqttClientService::setCallbackFunction(const std::string& topic, std::function<void(std::string)> callbackFunction) {
        CallbackFuctionsPerTopic[topic] = callbackFunction;
        if(ClientStarted) {
//        uint16_t packetId = Client->subscribe(topic, MQTT_NS::qos::at_least_once);
//        PackedIdToTopicMapping[packetId] = topic;
        }
    }

    void MqttClientService::connectClientStartCommunication() {
        async_mqtt::async_underlying_handshake(
                Client->next_layer(),
                Server, Port,
                [this](auto&&... args) {
                    handleUnderlyingHandshake(
                            std::forward<std::remove_reference_t<decltype(args)>>(args)...
                    );
                }
        );
//        Client->connect();
//        IoContext.run();
    }

    void MqttClientService::handleUnderlyingHandshake(async_mqtt::error_code errorCode) {
        std::cout << "underlying_handshake:" << errorCode.message() << std::endl;
        if(errorCode) return;
        Client->async_start(true,
                           std::uint16_t(0),
                           "openDigitalTwin",
                           std::nullopt,
                           "", //Username
                           "", //Password
                           [this](auto&&... args) {
                               handleStartResponse(std::forward<decltype(args)>(args)...);
                           }
                           );
    }

    void MqttClientService::handleStartResponse(async_mqtt::error_code ec,
                                                std::optional<async_mqtt::client<async_mqtt::protocol_version::v5, async_mqtt::protocol::mqtt>::connack_packet> connack_opt) {
        std::cout << "start:" << ec.message() << std::endl;
        if (ec) return;
        if (connack_opt) {
            std::cout << *connack_opt << std::endl;
            Client->async_publish(
                    "topic1",
                    "payload1",
                    async_mqtt::qos::at_least_once,
                    [this](auto&&... args) {
                        handlePublishResponse(
                                std::forward<std::remove_reference_t<decltype(args)>>(args)...
                        );
                    }
            );
//            Client.async_publish(
//                    *Client.acquire_unique_packet_id(), // sync version only works thread safe context
//                    "topic2",
//                    "payload2",
//                    async_mqtt::qos::at_least_once,
//                    [this](auto&&... args) {
//                        handle_publish_response(
//                                std::forward<std::remove_reference_t<decltype(args)>>(args)...
//                        );
//                    }
//            );
//            Client.async_publish(
//                    *Client.acquire_unique_packet_id(), // sync version only works thread safe context
//                    "topic3",
//                    "payload3",
//                    async_mqtt::qos::exactly_once,
//                    [this](auto&&... args) {
//                        handle_publish_response(
//                                std::forward<std::remove_reference_t<decltype(args)>>(args)...
//                        );
//                    }
//            );
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
}