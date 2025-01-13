//
// Created by Moritz Herzog on 06.08.24.
//

#ifndef DIGITALTWIN_MQTTCLIENTSERVICE_H
#define DIGITALTWIN_MQTTCLIENTSERVICE_H

#include <string>
#include <functional>
#include <map>
#include <memory>
#include <cstdint>
#include <coroutine>

#include <async_mqtt/all.hpp>


#include "../cpp_physical_twin_communication_global.h"

using client_t = async_mqtt::client<async_mqtt::protocol_version::v3_1_1,async_mqtt::protocol::mqtt>;

namespace PHYSICAL_TWIN_COMMUNICATION {
    class CPPPHYSICALTWINCOMMUNICATION_EXPORT MqttClientService {
    public:
        /**
         * Deleted, because this is not needed!
         */
        MqttClientService() = delete;
        /**
         *
         * @param server
         * @param port
         */
        MqttClientService(std::string server, std::string port);

        virtual ~MqttClientService();

        void sendValueToServer(std::string topic, std::string content);
        void addCallbackFunction(const std::string& topic, std::function<void(std::string)> callbackFunction);
        void addCallbackFunction(const std::string& topic, std::function<void(std::string)> callbackFunction, std::string valueForInit);

        void connectClientStartCommunication();

    private:
        void handleUnderlyingHandshake(async_mqtt::error_code errorCode);
        void handleStartResponse(async_mqtt::error_code ec, std::optional<client_t::connack_packet> connack_opt);
        void handlePublishResponse(async_mqtt::error_code ec, client_t::pubres_type pubres);
        void handleSubscribeResponse(async_mqtt::error_code ec, std::optional<client_t::suback_packet> suback_opt);
        void handleReceive(async_mqtt::error_code ec, async_mqtt::packet_variant pv);

        std::map<std::string,std::function<void(std::string)>> CallbackFuctionsPerTopic;
        std::map<uint16_t, std::string> PackedIdToTopicMapping;

        boost::asio::io_context ioContext;
        std::shared_ptr<client_t> Client;
        std::string Server;
        std::string Port;

        bool ClientStarted = false;
    };
}

#endif //DIGITALTWIN_MQTTCLIENTSERVICE_H
