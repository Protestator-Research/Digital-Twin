//
// Created by Moritz Herzog on 06.08.24.
//

#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <boost/asio/recycling_allocator.hpp>
#include <memory>
#include <iostream>

#include "MqttBrokerService.h"

#include <async_mqtt/all.hpp>

#include "Session.h"
#include "SubscriptionStorage.h"

namespace DIGITAL_TWIN_SERVER {

    MQTTBrokerService::MQTTBrokerService(boost::asio::io_context* ioc, unsigned serverPort, BrokerLimits limits, std::string serverCertPath, std::string serverCertPrivKeyPath) :
    Context(ioc),
    Acceptor(*ioc),
    Limits(limits),
    Tracker(std::make_shared<ConnectionTracker>(limits)),
    AcceptRetryTimer(*ioc)
    {
        ServerPort = serverPort;
        assert(!(!serverCertPath.empty() && serverCertPrivKeyPath.empty()));
        ServerCertPath = serverCertPath;
        ServerCertPrivKeyPath = serverCertPrivKeyPath;
        openAcceptor(serverPort);
    }

    MQTTBrokerService::MQTTBrokerService(boost::asio::io_context* ioc, std::string serverCertPath, std::string serverCertPrivKeyPath):
    MQTTBrokerService(ioc, 1883, BrokerLimits(), std::move(serverCertPath), std::move(serverCertPrivKeyPath))
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
        accept_one();
        Context->run();
    }

    void MQTTBrokerService::stop()
    {
        boost::asio::post(*Context, [this] {
            boost::system::error_code ec;
            AcceptRetryTimer.cancel();
            Acceptor.close(ec);
            Context->stop();
        });
    }

    namespace {
        // IPv4-mapped IPv6 addresses (::ffff:a.b.c.d) are counted like the plain IPv4 address.
        std::string normalizedIpKey(boost::asio::ip::address address) {
            if (address.is_v6() && address.to_v6().is_v4_mapped())
                address = boost::asio::ip::make_address_v4(boost::asio::ip::v4_mapped, address.to_v6());
            return address.to_string();
        }
    }

    void MQTTBrokerService::accept_one() {
        auto s = std::make_shared<Session>(Context, Subscriptions, authService, Limits);
        Acceptor.async_accept(s->lowest_layer(), [this, s](boost::system::error_code ec) {
            if (ec == boost::asio::error::operation_aborted)
                return;

            if (ec) {
                // The session is dropped (no leak). Back off briefly to avoid a busy loop, e.g. on EMFILE.
                std::cerr << "MQTT accept failed: " << ec.message() << std::endl;
                AcceptRetryTimer.expires_after(std::chrono::milliseconds(100));
                AcceptRetryTimer.async_wait([this](boost::system::error_code const& timerEc) {
                    if (!timerEc) accept_one();
                });
                return;
            }

            boost::system::error_code endpointEc;
            const auto remote = s->lowest_layer().remote_endpoint(endpointEc);
            if (endpointEc) {
                s->lowest_layer().close(endpointEc);
            } else {
                const auto ipKey = normalizedIpKey(remote.address());
                if (Tracker->tryAcquire(ipKey)) {
                    s->registerConnection(Tracker, ipKey);
                    s->start();
                } else {
                    std::cerr << "MQTT connection from " << ipKey << " rejected (connection limit)" << std::endl;
                    s->lowest_layer().close(endpointEc);
                }
            }
            accept_one();
        });
    }
}
