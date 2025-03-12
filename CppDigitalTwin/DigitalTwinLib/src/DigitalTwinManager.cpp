//
// Created by Moritz Herzog on 17.01.24.
//

#include "DigitalTwinManager.h"
#include "Model/DigitalTwinModel.h"

#include <BECommunicationService.h>
#include <Services/MqttClientService.h>
#include "AGILABackendImplementation/DigitalTwin.h"
#include <sysmlv2/entities/Element.h>
#include <MQTT/entities/DigitalTwinEntity.h>
#include <Exeptions/HttpException.h>
#include <MQTT/Topics.h>
#include <iostream>

namespace DigitalTwin {

    DigitalTwinManager::DigitalTwinManager(BACKEND_COMMUNICATION::CommunicationService *communicationService, PHYSICAL_TWIN_COMMUNICATION::MqttClientService* clientService, bool isClient) {
        BackendCommunicationService = communicationService;
        ClientService = clientService;
        IsClient = isClient;
    }

    DigitalTwinManager::~DigitalTwinManager() {

    }

    void DigitalTwinManager::downloadDigitalTwin(boost::uuids::uuid projectId, boost::uuids::uuid digitalTwinId) {
        try {
            auto digitalTwins = BackendCommunicationService->getAllDigitalTwinsForProjectWithId(projectId);

            std::cout<<"Number of Downloaded DTs from Backend for Project: "<<digitalTwins.size()<<std::endl;

            for (auto digitalTwin: digitalTwins)
                if (digitalTwin->getId() == digitalTwinId) {
                    std::cout<<"Found DT: "<<digitalTwin->getName() << std::endl;
                    auto returnValue = new Model::DigitalTwinModel(digitalTwin, this);
                    DigitalTwinModelMap.insert(std::make_pair(digitalTwin->getId(), returnValue));
                    std::cout<<"Generate MQTT Interface"<<std::endl;
                    generateMQTTInterface(returnValue);
                }
        } catch(BACKEND_COMMUNICATION::EXCEPTIONS::HTTPException& ex) {
            std::cout << ex.what() <<std::endl;
        }
    }

    std::vector<SysMLv2::Entities::Element *> DigitalTwinManager::downloadDigitalTwinModel(boost::uuids::uuid projectId, boost::uuids::uuid commitId) {
        return BackendCommunicationService->getAllElementsOfCommit(projectId,commitId);
    }

    DigitalTwin::Model::DigitalTwinModel* DigitalTwinManager::addDigitalTwinAndCreateModel(SysMLv2::Entities::DigitalTwin *digitalTwin) {
        Model::DigitalTwinModel* returnValue = new Model::DigitalTwinModel(digitalTwin,this);
        DigitalTwinModelMap.insert(std::make_pair(digitalTwin->getId(),returnValue));
        PHYSICAL_TWIN_COMMUNICATION::DigitalTwinEntity entity(digitalTwin->getId(), digitalTwin->parentProjectId());
        ClientService->sendValueToServer(PHYSICAL_TWIN_COMMUNICATION::CONNECT_TO_TWIN, entity.serialize());
        return returnValue;
    }

    void DigitalTwinManager::generateMQTTInterface(Model::DigitalTwinModel* digitalTwin) {
        std::string baseName = digitalTwin->digitalTwinName();
        for(const auto &elementName : digitalTwin->getElementStrings()) {
            ClientService->sendValueToServer(baseName + "/" + elementName, "testValue");
        }
    }

}