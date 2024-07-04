//
// Created by Moritz Herzog on 17.01.24.
//

#include "DigitalTwinManager.h"
#include "Model/DigitalTwinModel.h"

#include <BECommunicationService.h>
#include <SysMLv2Standard/entities/DigitalTwin.h>

namespace DigitalTwin {

    DigitalTwinManager::DigitalTwinManager(BACKEND_COMMUNICATION::CommunicationService *communicationService) {
        BackendCommunicationService = communicationService;
    }

    void DigitalTwinManager::addDigitalTwin(SysMLv2::Entities::DigitalTwin *digitalTwin) {
        DigitalTwinModelMap.insert(std::make_pair(digitalTwin->getId(), digitalTwin));
    }
}