//
// Created by Moritz Herzog on 17.01.24.
//

#include "DigitalTwinManager.h"
#include "Model/DigitalTwinModel.h"

#include <BECommunicationService.h>
#include <SysMLv2Standard/entities/DigitalTwin.h>
#include <SysMLv2Standard/entities/Element.h>

namespace DigitalTwin {

    DigitalTwinManager::DigitalTwinManager(BACKEND_COMMUNICATION::CommunicationService *communicationService) {
        BackendCommunicationService = communicationService;
    }

    void DigitalTwinManager::downloadDigitalTwin(boost::uuids::uuid projectId, boost::uuids::uuid digitalTwinId) {
        auto digitalTwins = BackendCommunicationService->getAllDigitalTwinsForProjectWithId(projectId);
        for(auto digitalTwin : digitalTwins)
            if(digitalTwin->getId()==digitalTwinId) {
                DigitalTwinModelMap.insert(std::make_pair(digitalTwin->getId(), Model::DigitalTwinModel(digitalTwin, this)));
            }
    }

    std::vector<SysMLv2::Entities::Element *>
    DigitalTwinManager::downloadDigitalTwinModel(boost::uuids::uuid projectId, boost::uuids::uuid commitId) {
        return BackendCommunicationService->getAllElementsOfCommit(projectId,commitId);
    }

}