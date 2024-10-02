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

#include <async_mqtt/all.hpp>


#include "../cpp_physical_twin_communication_global.h"

using client_t = async_mqtt::client<async_mqtt::protocol_version::v5,async_mqtt::protocol::mqtt>;

namespace PHYSICAL_TWIN_COMMUNICATION {
    class CPPPHYSICALTWINCOMMUNICATION_EXPORT MqttClientService {
    public:

        MqttClientService() = delete;
        MqttClientService(std::string server, std::string port);

        virtual ~MqttClientService();

        void sendValueToServer(std::string topic, std::string content);
        void setCallbackFunction(const std::string& topic, std::function<void(std::string)> callbackFunction);

        void connectClientStartCommunication();



    private:
        void handleUnderlyingHandshake(async_mqtt::error_code errorCode);
        void handleStartResponse(async_mqtt::error_code ec, std::optional<client_t::connack_packet> connack_opt);
        void handlePublishResponse(async_mqtt::error_code ec, client_t::pubres_type pubres);

        std::map<std::string,std::function<void(std::string)>> CallbackFuctionsPerTopic;
        std::map<uint16_t, std::string> PackedIdToTopicMapping;
        //boost::asio::io_context IoContext;
        //std::shared_ptr<mqtt::callable_overlay<mqtt::sync_client<mqtt::tcp_endpoint<boost::asio::basic_stream_socket<boost::asio::ip::tcp>, boost::asio::strand<boost::asio::io_context::basic_executor_type<std::allocator<void>, 0>>>>>> Client;
        //using packet_id_t = typename std::remove_reference_t<decltype(*Client)>::packet_id_t;

        boost::asio::io_context ioContext;
        std::shared_ptr<client_t> Client;
        std::string Server;
        std::string Port;

        bool ClientStarted = false;
    };
}

#endif //DIGITALTWIN_MQTTCLIENTSERVICE_H
