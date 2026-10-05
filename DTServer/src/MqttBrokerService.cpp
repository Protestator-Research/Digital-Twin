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
    Acceptor(*ioc)
    {
        ServerPort = serverPort;
        assert(!(!serverCertPath.empty() && serverCertPrivKeyPath.empty()));
        ServerCertPath = serverCertPath;
        ServerCertPrivKeyPath = serverCertPrivKeyPath;
        openAcceptor(serverPort);
    }

    MQTTBrokerService::MQTTBrokerService(boost::asio::io_context* ioc, std::string serverCertPath, std::string serverCertPrivKeyPath):
    MQTTBrokerService(ioc, 1883, std::move(serverCertPath), std::move(serverCertPrivKeyPath))
    {
    }

    void MQTTBrokerService::openAcceptor(unsigned port)
    {
        // Prefer a dual-stack (IPv6 + IPv4-mapped) listener, fall back to IPv4 only.
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v6(), static_cast<unsigned short>(port));
        boost::system::error_code ec;
        Acceptor.open(endpoint.protocol(), ec);
        if (!ec) Acceptor.set_option(boost::asio::ip::v6_only(false), ec);
        if (!ec) Acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
        if (ec) {
            boost::system::error_code ignored;
            Acceptor.close(ignored);
            endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), static_cast<unsigned short>(port));
            Acceptor.open(endpoint.protocol());
            Acceptor.set_option(boost::asio::socket_base::reuse_address(true));
        }
        Acceptor.bind(endpoint);
        Acceptor.listen();
    }

    void MQTTBrokerService::setUpTLS()
    {
    }

    void MQTTBrokerService::run()
    {
        SubscriptionStorage hub;
        accept_one(hub);
        Context->run();
    }

    void MQTTBrokerService::accept_one(SubscriptionStorage& hub) {
        auto s = new Session(Context, hub, authService);
        Acceptor.async_accept(s->lowest_layer(), [&, s](boost::system::error_code ec) {
            if (!ec) s->start();
            accept_one(hub);
        });
    }
}
