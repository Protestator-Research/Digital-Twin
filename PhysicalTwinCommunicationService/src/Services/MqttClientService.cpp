//
// Created by Moritz Herzog on 06.08.24.
//

#include "MqttClientService.h"

#include "../MQTT/entities/DigitalTwinEntity.h"
#include "../MQTT/Topics.h"

#include <iostream>
#include <async_mqtt/all.hpp>
#include <thread>

#include <boost/asio.hpp>
#include <boost/asio/async_result.hpp>

namespace PHYSICAL_TWIN_COMMUNICATION {


    MqttClientService::MqttClientService(std::string server, std::string port) :
        ioContext(boost::asio::io_context())
    {
        Server = server;
        Port = port;
        Client = client_t::create(ioContext.get_executor());
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

    void MqttClientService::addCallbackFunction(const std::string& topic, std::function<void(std::string)> callbackFunction, std::string valueForInit) {
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

            Client->async_publish(
                    *Client->acquire_unique_packet_id(),
                    topic,
                    valueForInit,
                    async_mqtt::qos::at_least_once,
                    [this](auto&&... args) {
                        handlePublishResponse(
                                std::forward<std::remove_reference_t<decltype(args)>>(args)...
                        );
                    }
            );
        }
    }

    void MqttClientService::connectClientStartCommunication() {
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
        ClientStarted=true;
    }

    void MqttClientService::handleStartResponse(async_mqtt::error_code ec,
                                                std::optional<client_t::connack_packet> connack_opt) {
        if (ec) return;
        if (connack_opt) {
            std::cout << *connack_opt << std::endl;
//            Client->async_publish(
//                    *Client->acquire_unique_packet_id(),
//                    CONNECT_TO_TWIN,
//                    "value",
//                    async_mqtt::qos::at_least_once,
//                    [this](auto&&... args) {
//                        handlePublishResponse(
//                                std::forward<std::remove_reference_t<decltype(args)>>(args)...
//                        );
//                    }
//            );
        }
    }

    void MqttClientService::handlePublishResponse(async_mqtt::error_code ec, client_t::pubres_type pubres) {
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
        if (ec) {
//            reconnect();
            return;
        }
        BOOST_ASSERT(pv);
        auto value = pv.get<async_mqtt::v3_1_1::publish_packet>();

        try {
            CallbackFuctionsPerTopic[value.topic()](value.payload());
        } catch(std::exception& ex) {
            std::cout << ex.what() << std::endl;
        }

        // next receive
        Client->async_recv(
                [this](auto&&... args) {
                    handleReceive(
                            std::forward<std::remove_reference_t<decltype(args)>>(args)...
                    );
                }
        );
    }

    void MqttClientService::addCallbackFunction(const std::string &topic, std::function<void(std::string)> callbackFunction) {
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
        }
    }
}