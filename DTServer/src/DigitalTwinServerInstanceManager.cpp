//
// Created by Moritz Herzog on 17.01.24.
//

#include "DigitalTwinServerInstanceManager.h"
#include <Model/DigitalTwinModel.h>
#include <BaseFuctions/StringExtention.hpp>
#include <MQTT/entities/DigitalTwinEntity.h>
#include <MQTT/Topics.h>
#include <iostream>
#include <utility>

namespace DIGITAL_TWIN_SERVER {
    DigitalTwinServerInstanceManager::DigitalTwinServerInstanceManager(int argc, char *argv[])
    {
        mapInstanceSettingsByArguments(argc,argv);
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
        BrokerService = new MQTTBrokerService(new boost::asio::io_context(),1883);
        ClientService = new PHYSICAL_TWIN_COMMUNICATION::MqttClientService("localhost","1883","digital-twin-server");
        DigitalTwinManager = new DigitalTwin::DigitalTwinManager(BackendCommunicationService, ClientService, true);
    }

    void DigitalTwinServerInstanceManager::runInstance() {
        std::thread mqttBrokerThread([this]() {
            BrokerService->run();
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // ClientService->start();

        BackendCommunicationService->setUserForLoginInBackend(ArgumentsMap[AGILA_USERNAME], ArgumentsMap[AGILA_PASSWORD]);

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

}