//
// Created by Moritz Herzog on 17.01.24.
//

#include "DigitalTwinServerInstanceManager.h"
#include <Model/DigitalTwinModel.h>
#include <MQTT/Topics.h>
#include <BaseFuctions/StringExtention.hpp>
#include <MQTT/entities/DigitalTwinEntity.h>
#include <MQTT/Topics.h>
#include <nlohmann/json.hpp>
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
        if(ErrorCode==EXIT_SUCCESS){
            delete BackendCommunicationService;
            delete DigitalTwinManager;
        }
    }

    void DigitalTwinServerInstanceManager::createInstance() {
        BackendCommunicationService = new BACKEND_COMMUNICATION::CommunicationService(
                ArgumentsMap[AGILA_URL],
                std::stoi(ArgumentsMap[AGILA_PORT]), "");

        const auto ioc = new boost::asio::io_context();

        BrokerService = new MQTTBrokerService(ioc, std::stoi(ArgumentsMap[INSTANCE_MQTT_PORT]));

        ClientService = new DigitalTwin::Communication::MqttClientService(ioc,"localhost", ArgumentsMap[INSTANCE_MQTT_PORT], "digital-twin-server");
        DigitalTwinManager = new DigitalTwin::DigitalTwinManager(BackendCommunicationService, ClientService, false);
    }

    void DigitalTwinServerInstanceManager::runInstance() {
        BackendCommunicationService->setUserForLoginInBackend(ArgumentsMap[AGILA_USERNAME], ArgumentsMap[AGILA_PASSWORD]);

        std::thread mqttBrokerThread([this]() {
            BrokerService->run();
        });
        std::thread mqttClientThread([this]() {
            ClientService->start();
        });

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
                    if(argVString.find(Arguments[i])!=std::string::npos)
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

            ArgumentsMap[AGILA_URL] = json["sysml"]["url"];
            ArgumentsMap[AGILA_PORT] = json["sysml"]["port"];
            ArgumentsMap[AGILA_USERNAME] = json["sysml"]["username"]; 
            ArgumentsMap[AGILA_PASSWORD] = json["sysml"]["password"];

            ArgumentsMap[INSTANCE_MQTT_PORT] = json["run"]["mqtt"]["port"];
            ArgumentsMap[INSTANCE_MQTT_CERT_CHAIN] = json["run"]["mqtt"]["cert_chain"];
            ArgumentsMap[INSTANCE_MQTT_CERT_PRIV] = json["run"]["mqtt"]["cert_private_key"];
        }
    }

    void DigitalTwinServerInstanceManager::createDTTopicAndCallback() {
        ClientService->publish(DigitalTwin::Communication::CONNECT_TO_TWIN, DigitalTwin::Communication::DigitalTwinEntity().serialize());
        ClientService->subscribe(DigitalTwin::Communication::CONNECT_TO_TWIN,[this]([[maybe_unused]] std::string topic,std::string payload)->void {
            const auto& dtEntity = DigitalTwin::Communication::DigitalTwinEntity(payload);
            DigitalTwinManager->downloadDigitalTwin(dtEntity.projectId(),dtEntity.digitalTwinId());
        });
    }
}
