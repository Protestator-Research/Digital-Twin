//
// Created by Moritz Herzog on 06.08.24.
//

#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <boost/asio/recycling_allocator.hpp>
#include <memory>

#include "MqttBrokerService.h"

#include <async_mqtt/all.hpp>

#include "Session.h"
#include "SubscriptionStorage.h"

namespace DIGITAL_TWIN_SERVER {

    MQTTBrokerService::MQTTBrokerService(boost::asio::io_context* ioc, unsigned serverPort, std::string serverCertPath, std::string serverCertPrivKeyPath) :
    Context(ioc),
    Acceptor(*ioc,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), serverPort))
    {
        ServerPort = serverPort;
        assert(!(!serverCertPath.empty() && serverCertPrivKeyPath.empty()));
        ServerCertPath = serverCertPath;
        ServerCertPrivKeyPath = serverCertPrivKeyPath;
    }

    MQTTBrokerService::MQTTBrokerService(boost::asio::io_context* ioc, std::string serverCertPath, std::string serverCertPrivKeyPath):
    Context(ioc),
    Acceptor(*ioc,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 1883))
    {
        assert(!(!serverCertPath.empty() && serverCertPrivKeyPath.empty()));
        ServerCertPath = serverCertPath;
        ServerCertPrivKeyPath = serverCertPrivKeyPath;
    }

    void MQTTBrokerService::setUpTLS()
    {
    }

    void MQTTBrokerService::run()
    {
        accept_one();
    }

    void MQTTBrokerService::accept_one() {
        SubscriptionStorage hub;

        std::function<void()> do_accept;
        do_accept = [this, &hub, do_accept]()  {
            auto s = std::make_shared<Session>(Context->get_executor(), hub, authService);
            Acceptor.async_accept(s->lowest_layer(), [&, s](boost::system::error_code ec) {
                if (!ec) s->start();
                do_accept();
            });
        };
        do_accept();

        Context->run();
    }
}
