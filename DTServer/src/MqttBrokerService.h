//
// Created by Moritz Herzog on 06.08.24.
//

#ifndef DIGITALTWIN_MQTTBROKERSERVICE_H
#define DIGITALTWIN_MQTTBROKERSERVICE_H

#include <string>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

namespace DIGITAL_TWIN_SERVER {
    /**
     * @class MQTTBrokerService
     * @author Moritz Herzog <moritz.herzog@protestator-research.com>
     * @version 2.0
     */
    class MQTTBrokerService {
    public:
        MQTTBrokerService() = default;

        MQTTBrokerService(unsigned serverPort, std::string serverCertPath = "", std::string serverCertPrivKeyPath = "");
        MQTTBrokerService(std::string serverCertPath, std::string serverCertPrivKeyPath);

        void setUpTLS();
        void run();

    private:
        boost::asio::io_context& IO_Context;
        boost::asio::ssl::context TLS_Context{ boost::asio::ssl::context::tls_server };
        boost::asio::ip::tcp::acceptor Acceptor;

        unsigned ServerPort = 1883;
        std::string ServerCertPath = "";
        std::string ServerCertPrivKeyPath = "";
    };
}

#endif //DIGITALTWIN_MQTTBROKERSERVICE_H
