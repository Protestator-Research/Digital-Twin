//
// Created by Moritz Herzog on 06.08.24.
//

#ifndef DIGITALTWIN_MQTTBROKERSERVICE_H
#define DIGITALTWIN_MQTTBROKERSERVICE_H

#include <mqtt/config.hpp>
#include <mqtt/setup_log.hpp>
#include <mqtt/broker/broker.hpp>
#include <boost/program_options.hpp>
#include <boost/format.hpp>

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>

namespace as = boost::asio;

using con_t = MQTT_NS::server<>::endpoint_t;
using con_sp_t = std::shared_ptr<con_t>;

namespace PHYSICAL_TWIN_COMMUNICATION {
    class MQTTBrokerService {
    public:
        MQTTBrokerService() = delete;
        MQTTBrokerService(as::io_context& ioc_accept,
                          std::function<as::io_context&()> ioc_con_getter,
                          MQTT_NS::broker::broker_t& broker,
                          uint16_t port);

        void listen();

        MQTT_NS::broker::broker_t& getBroker() const;

        void close();

        static void runBroker(uint16_t port);
    private:
        MQTT_NS::server<> Server;
        MQTT_NS::broker::broker_t& Broker;
    };
}

#endif //DIGITALTWIN_MQTTBROKERSERVICE_H
