//
// Created by Moritz Herzog on 13.12.23.
//
//---------------------------------------------------------
// Constants, Definitions, Pragmas
//---------------------------------------------------------

//---------------------------------------------------------
// External Classes
//---------------------------------------------------------
#include <utility>

//---------------------------------------------------------
// Internal Classes
//---------------------------------------------------------
#include "CommunicationService.h"


BACKEND_COMMUNICATION::CommunicationService::CommunicationService(std::string serverAddress, unsigned int port) {
    ServerAddress = std::move(serverAddress);
    Port = port;
}

BACKEND_COMMUNICATION::CommunicationService::CommunicationService(std::string serverAddress) {
    ServerAddress = std::move(serverAddress);
}

std::vector<BACKEND_COMMUNICATION::ENTITIES::Element>BACKEND_COMMUNICATION::CommunicationService::getAllElements(uuid_t commitId, uuid_t projectId) {

    return std::vector<ENTITIES::Element>();
}

std::vector<BACKEND_COMMUNICATION::ENTITIES::Project> BACKEND_COMMUNICATION::CommunicationService::getAllProjects() {
    return std::vector<ENTITIES::Project>();
}


