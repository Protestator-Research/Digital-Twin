//
// Created by Moritz Herzog on 17.01.24.
//

#include "DigitalTwinServerInstanceManager.h"
#include <Model/DigitalTwinModel.h>
#include <MQTT/Topics.h>
#include <BaseFuctions/StringExtention.hpp>
#include <../../PhysicalTwinCommunicationService/src/MQTT/entities/DigitalTwinEntity.h>
#include <MQTT/Topics.h>
#include <nlohmann/json.hpp>
#include <boost/asio/post.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <sstream>
#include <iostream>
#include <utility>
#include <fstream>
#include <string>

namespace DIGITAL_TWIN_SERVER {
    DigitalTwinServerInstanceManager::DigitalTwinServerInstanceManager(int argc, char *argv[])
    {
        mapInstanceSettingsByArguments(argc,argv);
        openConfigFileIfExist();
    }

    DigitalTwinServerInstanceManager::~DigitalTwinServerInstanceManager() {
        if (ClientService)
            ClientService->stop();
        if (BrokerService)
            BrokerService->stop();
        DownloadPool.join();
        if(ErrorCode==EXIT_SUCCESS){
            delete BackendCommunicationService;
            delete DigitalTwinManager;
        }
        delete ClientService;
        delete BrokerService;
    }

    void DigitalTwinServerInstanceManager::createInstance() {
        BackendCommunicationService = new BACKEND_COMMUNICATION::CommunicationService(
                ArgumentsMap[AGILA_URL],
                std::stoi(ArgumentsMap[AGILA_PORT]), "");

        BrokerContext = std::make_unique<boost::asio::io_context>();
        BrokerService = new MQTTBrokerService(BrokerContext.get(), std::stoi(ArgumentsMap[INSTANCE_MQTT_PORT]), Limits);

        // The client service owns its own io_context and thread, it is not shared with the broker.
        ClientService = new DigitalTwin::Communication::MqttClientService("localhost", ArgumentsMap[INSTANCE_MQTT_PORT], "digital-twin-server");
        DigitalTwinManager = new DigitalTwin::DigitalTwinManager(BackendCommunicationService, ClientService, false);
    }

    void DigitalTwinServerInstanceManager::runInstance() {
        BackendCommunicationService->setUserForLoginInBackend(ArgumentsMap[AGILA_USERNAME], ArgumentsMap[AGILA_PASSWORD]);

        std::thread mqttBrokerThread([this]() {
            BrokerService->run();
        });
        ClientService->start();

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        createDTTopicAndCallback();

        mqttBrokerThread.join();
    }

    int DigitalTwinServerInstanceManager::getRunTimeCode() {
        return ErrorCode;
    }

    void DigitalTwinServerInstanceManager::destroyOnError() {

    }

    void DigitalTwinServerInstanceManager::mapInstanceSettingsByArguments(int argc, char *argv[]) {
        if(argc > 1) {

            for(int i = 0; i<ARGUMENTS_SIZE; i++) {
                for(int j = 0; j<argc; j++) {
                    std::string argVString = std::string(argv[j]);
                    if(argVString.find(Arguments[i])!=std::string::npos && j + 1 < argc)
                        ArgumentsMap.insert(std::make_pair<ARGUMENTS, std::string>(ARGUMENTS(i), std::string(argv[j + 1])));
                }
            }

            for(int i = 0; i<ARGUMENTS_SIZE; i++)
                if(ArgumentsMap.count(ARGUMENTS(i))<1)
                    ArgumentsMap.insert(std::make_pair<ARGUMENTS,std::string>(ARGUMENTS(i), std::string(DefaultValueForArgument[i])));

        }
        else
            for(int i = 0; i<ARGUMENTS_SIZE; i++)
                ArgumentsMap.insert(std::make_pair<ARGUMENTS,std::string>(ARGUMENTS(i), std::string(DefaultValueForArgument[i])));
    }

    void DigitalTwinServerInstanceManager::openConfigFileIfExist()
    {
        if (!ArgumentsMap[ARGUMENTS::INSTANCE_CONFIG_FILE_PATH].empty()) {
            std::ifstream file(ArgumentsMap[ARGUMENTS::INSTANCE_CONFIG_FILE_PATH].c_str());
            std::stringstream buffer;
            buffer << file.rdbuf();

            nlohmann::json json = nlohmann::json::parse(buffer.str());

            const auto sysml = json.value("sysml", nlohmann::json::object());
            ArgumentsMap[AGILA_URL] = sysml.value("url", ArgumentsMap[AGILA_URL]);
            ArgumentsMap[AGILA_PORT] = sysml.value("port", ArgumentsMap[AGILA_PORT]);
            ArgumentsMap[AGILA_USERNAME] = sysml.value("username", ArgumentsMap[AGILA_USERNAME]);
            ArgumentsMap[AGILA_PASSWORD] = sysml.value("password", ArgumentsMap[AGILA_PASSWORD]);

            const auto run = json.value("run", nlohmann::json::object());
            const auto mqtt = run.value("mqtt", nlohmann::json::object());
            ArgumentsMap[INSTANCE_MQTT_PORT] = mqtt.value("port", ArgumentsMap[INSTANCE_MQTT_PORT]);
            ArgumentsMap[INSTANCE_MQTT_CERT_CHAIN] = mqtt.value("cert_chain", ArgumentsMap[INSTANCE_MQTT_CERT_CHAIN]);
            ArgumentsMap[INSTANCE_MQTT_CERT_PRIV] = mqtt.value("cert_private_key", ArgumentsMap[INSTANCE_MQTT_CERT_PRIV]);

            // Optional limits, missing keys keep their defaults.
            const auto limits = mqtt.value("limits", nlohmann::json::object());
            Limits.MaxPacketSize = limits.value("max_packet_size", Limits.MaxPacketSize);
            Limits.MaxConnections = limits.value("max_connections", Limits.MaxConnections);
            Limits.MaxConnectionsPerIp = limits.value("max_connections_per_ip", Limits.MaxConnectionsPerIp);
            Limits.MaxSubscriptionsPerSession = limits.value("max_subscriptions_per_session", Limits.MaxSubscriptionsPerSession);
            Limits.MaxPendingSendBytes = limits.value("max_pending_send_bytes", Limits.MaxPendingSendBytes);
            Limits.ConnectTimeoutSeconds = limits.value("connect_timeout_s", Limits.ConnectTimeoutSeconds);
            Limits.ServerKeepAliveSeconds = limits.value("server_keep_alive_s", Limits.ServerKeepAliveSeconds);
        }
    }

    void DigitalTwinServerInstanceManager::createDTTopicAndCallback() {
        const auto subscriptionFunction = [this]([[maybe_unused]] std::string topic, std::string payload)->void {
            boost::asio::post(DownloadPool, [this, payload = std::move(payload)]() {
                boost::uuids::uuid digitalTwinId{};
                bool registered = false;
                try {
                    const DigitalTwin::Communication::DigitalTwinEntity dtEntity(payload);
                    digitalTwinId = dtEntity.digitalTwinId();
                    {
                        std::lock_guard lg(DownloadsInProgressMutex);
                        registered = DownloadsInProgress.insert(digitalTwinId).second;
                    }
                    if (!registered) {
                        std::cerr << "Download of digital twin " << boost::uuids::to_string(digitalTwinId)
                                  << " already in progress, request skipped." << std::endl;
                        return;
                    }
                    DigitalTwinManager->downloadDigitalTwin(dtEntity.projectId(), digitalTwinId);
                } catch (std::exception const& e) {
                    std::cerr << "Error while downloading digital twin: " << e.what() << std::endl;
                } catch (...) {
                    std::cerr << "Unknown error while downloading digital twin." << std::endl;
                }
                if (registered) {
                    std::lock_guard lg(DownloadsInProgressMutex);
                    DownloadsInProgress.erase(digitalTwinId);
                }
            });
        };

        ClientService->subscribe(DigitalTwin::Communication::CONNECT_TO_TWIN,subscriptionFunction);
    }
}
