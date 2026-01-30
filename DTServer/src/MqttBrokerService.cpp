//
// Created by Moritz Herzog on 06.08.24.
//

#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <boost/asio/recycling_allocator.hpp>
#include <memory>
#include <async_mqtt/endpoint.hpp>

#include "MqttBrokerService.h"




namespace DIGITAL_TWIN_SERVER {

    MQTTBrokerService::MQTTBrokerService(unsigned serverPort, std::string serverCertPath, std::string serverCertPrivKeyPath)
    {
        ServerPort = serverPort;
        assert(("Setting the servers SSL certificate but no private key is not allowed.", !serverCertPath.empty() && serverCertPrivKeyPath.empty()));
        ServerCertPath = serverCertPath;
        ServerCertPrivKeyPath = serverCertPrivKeyPath;
    }

    MQTTBrokerService::MQTTBrokerService(std::string serverCertPath, std::string serverCertPrivKeyPath)
    {
        assert(("Setting the servers SSL certificate but no private key is not allowed.", !serverCertPath.empty() && serverCertPrivKeyPath.empty()));
        ServerCertPath = serverCertPath;
        ServerCertPrivKeyPath = serverCertPrivKeyPath;
    }

    void MQTTBrokerService::setUpTLS()
    {
    }

    void MQTTBrokerService::run()
    {
    }
}
