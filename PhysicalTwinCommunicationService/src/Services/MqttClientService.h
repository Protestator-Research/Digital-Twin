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

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/strand.hpp>

#include "../cpp_physical_twin_communication_global.h"

using client_t = async_mqtt::client<async_mqtt::protocol_version::v5,async_mqtt::protocol::mqtt>;

namespace PHYSICAL_TWIN_COMMUNICATION {
    class CPPPHYSICALTWINCOMMUNICATION_EXPORT MqttClientService {
    public:
        /**
         * Deleted, because this is not needed!
         */
        MqttClientService() = delete;
        /**
         * Constructor needed for the connection to a Server.
         * @param server The Server URL or IP
         * @param port The Port on the server, where the DT Server is running.
         */
        MqttClientService(std::string server, std::string port, std::string clientId);

        virtual ~MqttClientService();

        void start();
        void stop();

        void publish(std::string topic, std::string payload, async_mqtt::qos qos=async_mqtt::qos::at_most_once);
        std::future<std::string> request(std::string topic, std::string payload, std::string respTopic);



    private:
        boost::asio::awaitable<void> run();
        static std::vector<uint8_t> makeCorrelationData();
        static std::optional<std::string> extractCorrelationKey(async_mqtt::v5::publish_packet const& packet);

        boost::asio::io_context IoContext;
        std::string Server;
        std::string Port;
        std::string ClientId;
        std::chrono::seconds KeepAlive;

        boost::asio::strand<boost::asio::any_io_executor> Strand;

        std::thread WorkerThread;
        async_mqtt::client<async_mqtt::protocol_version::v5, async_mqtt::protocol::mqtt> Client;

        std::atomic<bool> ClientStarted;
        std::atomic<bool> Connected;

        std::unordered_map<std::string, std::function<void(std::string topic, std::string payload)>> Subscriptions;
        std::unordered_map<std::string, std::promise<std::string>> Pending;
        bool HasResponseSubscription{false};
    };
}

#endif //DIGITALTWIN_MQTTCLIENTSERVICE_H
