//
// Created by Moritz Herzog on 17.01.24.
//

#include "DigitalTwinServerInstanceManager.h"
#include <Model/DigitalTwinModel.h>
#include <BaseFuctions/StringExtention.hpp>
#include <entities/DigitalTwinEntity.h>
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
            delete PhysicalTwinCommunicationService;
        }
    }

    void DigitalTwinServerInstanceManager::createInstance() {
        BackendCommunicationService = new BACKEND_COMMUNICATION::CommunicationService(
                ArgumentsMap[AGILA_URL],
                std::stoi(ArgumentsMap[AGILA_PORT]));
        PhysicalTwinCommunicationService = new PHYSICAL_TWIN_COMMUNICATION::CommunicationService(ArgumentsMap[INSTANCE_MQTT_PORT]);
        DigitalTwinManager = new DigitalTwin::DigitalTwinManager(BackendCommunicationService, PhysicalTwinCommunicationService->getClientService(), true);

    }

    void DigitalTwinServerInstanceManager::runInstance() {
        PhysicalTwinCommunicationService->startThreads();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        PhysicalTwinCommunicationService->addObservationCallbackForTopic("connectToTwin",[&](std::string value) {
            if(value.empty()) {
                PHYSICAL_TWIN_COMMUNICATION::DigitalTwinEntity entity;
                PhysicalTwinCommunicationService->publishMQTTMessage("connectToTwin",entity.serialize());
            } else {
                PHYSICAL_TWIN_COMMUNICATION::DigitalTwinEntity entity(value);
                DigitalTwinManager->downloadDigitalTwin(entity.projectId(),entity.digitalTwinId());
            }
        },PHYSICAL_TWIN_COMMUNICATION::DigitalTwinEntity().serialize());

        BackendCommunicationService->setUserForLoginInBackend(ArgumentsMap[AGILA_USERNAME], ArgumentsMap[AGILA_PASSWORD]);

        PhysicalTwinCommunicationService->joinThreads();
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