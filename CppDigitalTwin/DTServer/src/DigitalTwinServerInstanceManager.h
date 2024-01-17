//
// Created by Moritz Herzog on 17.01.24.
//

#pragma once

#include <string>
#include <cstdlib>

#include <BECommunicationService.h>
#include <DigitalTwinManager.h>
#include <PTCommunicationService.h>


namespace DIGITAL_TWIN_SERVER {
    class DigitalTwinServerInstanceManager {
    public:
        DigitalTwinServerInstanceManager() = default;
        virtual ~DigitalTwinServerInstanceManager();

        void createInstance();
        void runInstance();

        void destroyOnError();

        int getRunTimeCode();
    private:
        BACKEND_COMMUNICATION::CommunicationService* BackendCommunicationService = nullptr;
        DIGITAL_TWIN_LIB::DigitalTwinManager* DigitalTwinManager = nullptr;
        PHYSICAL_TWIN_COMMUNICATION::CommunicationService* PhysicalTwinCommunicationService = nullptr;

        std::string AgilaBackendServerAdress = "cps-agila.cs.rptu.de";
        int AgilaBackendPort = 8080;

        int ErrorCode = EXIT_SUCCESS;
    };
}