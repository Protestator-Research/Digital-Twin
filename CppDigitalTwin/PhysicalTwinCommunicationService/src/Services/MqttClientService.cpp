//
// Created by Moritz Herzog on 06.08.24.
//

#include "MqttClientService.h"

#include "../entities/DigitalTwinEntity.h"

#include <iostream>
#include <thread>


namespace PHYSICAL_TWIN_COMMUNICATION {

    MqttClientService::MqttClientService(std::string server, std::string port)
    {
        // Create no TLS client
        Client = MQTT_NS::make_sync_client(IoContext, server, port);

        auto disconnect = [&] {
//            if (++count == 5) Client->disconnect();
        };

        // Setup Client
        Client->set_client_id("DigitalTwinClient");
        Client->set_clean_session(true);

        // Setup handlers
        Client->set_connack_handler(
                [&]
                        (bool sp, MQTT_NS::connect_return_code connack_return_code){
                    std::cout << "Connack handler called" << std::endl;
                    std::cout << "Session Present: " << std::boolalpha << sp << std::endl;
                    std::cout << "Connack Return Code: "
                              << MQTT_NS::connect_return_code_to_str(connack_return_code) << std::endl;
                    if (connack_return_code == MQTT_NS::connect_return_code::accepted) {
                        uint16_t pid_sub = Client->subscribe("connectToTwin", MQTT_NS::qos::at_most_once);
                        PackedIdToTopicMapping[pid_sub]="connectToTwin";
                    }
                    return true;
                });
        Client->set_close_handler(
                []
                        (){
                    std::cout << "closed." << std::endl;
                });
        Client->set_error_handler(
                []
                        (MQTT_NS::error_code ec){
                    std::cout << "error: " << ec.message() << std::endl;
                });
        Client->set_puback_handler(
                [&]
                        (packet_id_t packet_id){
                    std::cout << "puback received. packet_id: " << packet_id << std::endl;
                    disconnect();
                    return true;
                });
        Client->set_pubrec_handler(
                []
                        (packet_id_t packet_id){
                    std::cout << "pubrec received. packet_id: " << packet_id << std::endl;
                    return true;
                });
        Client->set_pubcomp_handler(
                [&]
                        (packet_id_t packet_id){
                    std::cout << "pubcomp received. packet_id: " << packet_id << std::endl;
                    disconnect();
                    return true;
                });
        Client->set_suback_handler(
                [&]
                        (packet_id_t packet_id, std::vector<MQTT_NS::suback_return_code> results){
                    std::cout << "suback received. packet_id: " << packet_id << std::endl;
                    for (auto const& e : results) {
                        std::cout << "[client] subscribe result: " << e << std::endl;
                    }
                    std::string topic = PackedIdToTopicMapping[packet_id];
                    if(!topic.empty()) {
                        CallbackFuctionsPerTopic[topic]("");
                    }
                    return true;
                });
        Client->set_publish_handler(
                [this]
                        (MQTT_NS::optional<packet_id_t> packet_id,
                         MQTT_NS::publish_options pubopts,
                         MQTT_NS::buffer topic_name,
                         MQTT_NS::buffer contents){
                    std::cout << "publish received."
                              << " dup: "    << pubopts.get_dup()
                              << " qos: "    << pubopts.get_qos()
                              << " retain: " << pubopts.get_retain() << std::endl;
                    if (packet_id)
                        std::cout << "packet_id: " << *packet_id << std::endl;
                    std::cout << "topic_name: " << topic_name << std::endl;
                    std::cout << "contents: " << contents << std::endl;

                    CallbackFuctionsPerTopic[std::string(topic_name)](std::string(contents));

                    return true;
                });



    }

    MqttClientService::~MqttClientService() {

    }

    void MqttClientService::sendValueToServer(std::string topic, std::string value) {
        Client->publish(Client->acquire_unique_packet_id(), topic, value);
    }

    void MqttClientService::setCallbackFunction(const std::string& topic, std::function<void(std::string)> callbackFunction) {
        CallbackFuctionsPerTopic[topic] = callbackFunction;
        uint16_t packetId = Client->subscribe(topic, MQTT_NS::qos::at_least_once);
        PackedIdToTopicMapping[packetId] = topic;
    }

    void MqttClientService::connectClientStartCommunication() {

        CallbackFuctionsPerTopic["connectToTwin"] = [&](std::string value) {
            if(value.empty()) {
                DigitalTwinEntity entity;
                Client->publish("connectToTwin",entity.serialize());
            } else {
                std::cout<<"Message Received"<<std::endl;
            }
        };

        Client->connect();
        IoContext.run();
    }
}