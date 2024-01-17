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
#include "BECommunicationService.h"

#include "APIImplementations/SysMLAPIImplementation.h"

namespace BACKEND_COMMUNICATION {
    CommunicationService::CommunicationService(std::string serverAddress, unsigned int port) {
        ServerAddress = std::move(serverAddress);
        Port = port;
        SysMLAPIImplementation::connectToServer(REST_PROTOCOL + ServerAddress + ":" + std::to_string(Port) + ENTRY_URI);
    }

    CommunicationService::CommunicationService(std::string serverAddress) {
        ServerAddress = std::move(serverAddress);
    }

    std::vector<ENTITIES::Element> CommunicationService::getAllElements(uuid_t commitId, uuid_t projectId) {

        return std::vector<ENTITIES::Element>();
    }

    std::vector<ENTITIES::Project> CommunicationService::getAllProjects() {
        return std::vector<ENTITIES::Project>();
    }

    ENTITIES::DigitalTwin CommunicationService::getDigitalTwinWithID(unsigned char *digitalTwinId, unsigned char *projectId) {
        return ENTITIES::DigitalTwin();
    }

    std::vector<ENTITIES::Branch> CommunicationService::getAllBranchesForProjectWithID(unsigned char *projectId) {
        return std::vector<ENTITIES::Branch>();
    }

    ENTITIES::Commit CommunicationService::getCommitWithId(unsigned char *projectId, unsigned char *commitId) {
        return ENTITIES::Commit();
    }

    bool CommunicationService::setUserForLoginInBackend(std::string username, std::string password) {
        BarrierString = SysMLAPIImplementation::loginUserWithPassword(username,password);
        return !BarrierString.empty();
    }
}