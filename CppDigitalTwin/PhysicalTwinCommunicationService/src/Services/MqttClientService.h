//
// Created by Moritz Herzog on 06.08.24.
//

#ifndef DIGITALTWIN_MQTTCLIENTSERVICE_H
#define DIGITALTWIN_MQTTCLIENTSERVICE_H

#include <string>
#include <functional>
#include <map>
#include <mqtt_client_cpp.hpp>
#include <memory>
#include <cstdint>



namespace PHYSICAL_TWIN_COMMUNICATION {
    class MqttClientService {
    public:
        MqttClientService() = delete;
        MqttClientService(std::string server, std::string port);

        virtual ~MqttClientService();

        void sendValueToServer(std::string topic, std::string content);
        void setCallbackFunction(const std::string& topic, std::function<void(std::string)> callbackFunction);

        void connectClientStartCommunication();

    private:
        std::map<std::string,std::function<void(std::string)>> CallbackFuctionsPerTopic;
        std::map<uint16_t, std::string> PackedIdToTopicMapping;
        boost::asio::io_context IoContext;
        std::shared_ptr<mqtt::callable_overlay<mqtt::sync_client<mqtt::tcp_endpoint<boost::asio::basic_stream_socket<boost::asio::ip::tcp>, boost::asio::strand<boost::asio::io_context::basic_executor_type<std::allocator<void>, 0>>>>>> Client;
        using packet_id_t = typename std::remove_reference_t<decltype(*Client)>::packet_id_t;
    };
}

#endif //DIGITALTWIN_MQTTCLIENTSERVICE_H
