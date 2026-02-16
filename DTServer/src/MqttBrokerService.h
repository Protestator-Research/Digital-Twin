//
// Created by Moritz Herzog on 06.08.24.
//

#ifndef DIGITALTWIN_MQTTBROKERSERVICE_H
#define DIGITALTWIN_MQTTBROKERSERVICE_H

#include <string>
#include <unordered_set>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "Session.h"
#include "AuthenticationService.h"

namespace DIGITAL_TWIN_SERVER {
    /**
     * @class MQTTBrokerService
     * @author Moritz Herzog <moritz.herzog@protestator-research.com>
     * @version 2.0
     */

    struct Hub {
        std::unordered_set<std::shared_ptr<Session>> sessions;

        void join(std::shared_ptr<Session> const& s) { sessions.insert(s); }
        void leave(std::shared_ptr<Session> const& s) { sessions.erase(s); }

        // void broadcast_qos0(std::string const& topic, std::string const& payload) {
        //     for (auto const& s : sessions) {
        //         //if (s) s->send_qos0_publish(topic, payload);
        //     }
        // }
    };

    class MQTTBrokerService {
    public:
        explicit MQTTBrokerService(boost::asio::io_context* ioc, unsigned serverPort, std::string serverCertPath = "", std::string serverCertPrivKeyPath = "");
        explicit MQTTBrokerService(boost::asio::io_context* ioc, std::string serverCertPath, std::string serverCertPrivKeyPath);

        void setUpTLS();
        void run();

    private:
        void accept_one();

        boost::asio::io_context* Context;
        boost::asio::ssl::context TLS_Context{ boost::asio::ssl::context::tls_server };
        boost::asio::ip::tcp::acceptor Acceptor;
        unsigned ServerPort = 1883;
        std::string ServerCertPath = "";
        std::string ServerCertPrivKeyPath = "";
        AuthenticationService authService;
    };
}

#endif //DIGITALTWIN_MQTTBROKERSERVICE_H
