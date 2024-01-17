//
// Created by Moritz Herzog on 17.01.24.
//

#include "DigitalTwinServerInstanceManager.h"

namespace DIGITAL_TWIN_SERVER {
    DigitalTwinServerInstanceManager::~DigitalTwinServerInstanceManager() {
        if(ErrorCode==EXIT_SUCCESS){

        }
    }

    void DigitalTwinServerInstanceManager::createInstance() {

    }

    void DigitalTwinServerInstanceManager::runInstance() {

    }

    int DigitalTwinServerInstanceManager::getRunTimeCode() {
        return ErrorCode;
    }

    void DigitalTwinServerInstanceManager::destroyOnError() {

    }


}