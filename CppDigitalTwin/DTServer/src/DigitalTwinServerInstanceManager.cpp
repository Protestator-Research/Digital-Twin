//
// Created by Moritz Herzog on 17.01.24.
//

#include "DigitalTwinServerInstanceManager.h"

namespace DIGITAL_TWIN_SERVER {
    DigitalTwinServerInstanceManager::~DigitalTwinServerInstanceManager() {
        if(ErrorCode==EXIT_SUCCESS){
            delete BackendCommunicationService;
            delete DigitalTwinManager;
            delete PhysicalTwinCommunicationService;
        }
    }

    void DigitalTwinServerInstanceManager::createInstance() {
        BackendCommunicationService = new BACKEND_COMMUNICATION::CommunicationService(AgilaBackendServerAdress,AgilaBackendPort);
        DigitalTwinManager = new DIGITAL_TWIN_LIB::DigitalTwinManager();
        PhysicalTwinCommunicationService = new PHYSICAL_TWIN_COMMUNICATION::CommunicationService();
    }

    void DigitalTwinServerInstanceManager::runInstance() {
        BackendCommunicationService->setUserForLoginInBackend("admin@cps.de", "admin");
    }

    int DigitalTwinServerInstanceManager::getRunTimeCode() {
        return ErrorCode;
    }

    void DigitalTwinServerInstanceManager::destroyOnError() {

    }


}