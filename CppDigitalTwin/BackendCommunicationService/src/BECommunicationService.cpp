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
#include <iostream>
#include <SysMLv2Standard/entities/Project.h>

//---------------------------------------------------------
// Internal Classes
//---------------------------------------------------------
#include "BECommunicationService.h"
#include "Exeptions/ConnectionError.h"

#include "APIImplementations/SysMLAPIImplementation.h"

namespace BACKEND_COMMUNICATION {
    CommunicationService::CommunicationService(std::string serverAddress, unsigned int port) :
        ServerAddress(serverAddress),
        Port(port)
    {
        std::cout<<"Connecting to Server: "<<ServerAddress<<":"<<Port<<std::endl;

        if(Port==443)
            REST_PROTOCOL="https://";

        if(!SysMLAPIImplementation::connectToServer(REST_PROTOCOL + ServerAddress + ":" + std::to_string(Port) + ENTRY_URI))
            throw EXCEPTIONS::ConnectionError(EXCEPTIONS::CONNECTION_ERROR_TYPE::COULD_NOT_CONNECT);
    }

    CommunicationService::CommunicationService(std::string serverAddress) {
        ServerAddress = std::move(serverAddress);
    }

//    std::vector<ENTITIES::Element> CommunicationService::getAllElements(uuid_t commitId, uuid_t projectId) {
//
//        return std::vector<ENTITIES::Element>();
//    }

    std::vector<SysMLv2::Entities::IEntity*> CommunicationService::getAllProjects() {
        return SysMLAPIImplementation::getAllProjects(BarrierString);
    }
//
//    ENTITIES::DigitalTwin CommunicationService::getDigitalTwinWithID(unsigned char *digitalTwinId, unsigned char *projectId) {
//        return ENTITIES::DigitalTwin();
//    }
//
//    std::vector<ENTITIES::Branch> CommunicationService::getAllBranchesForProjectWithID(unsigned char *projectId) {
//        return std::vector<ENTITIES::Branch>();
//    }
//
//    ENTITIES::Commit CommunicationService::getCommitWithId(unsigned char *projectId, unsigned char *commitId) {
//        return ENTITIES::Commit();
//    }

    bool CommunicationService::setUserForLoginInBackend(std::string username, std::string password) {
        BarrierString = SysMLAPIImplementation::loginUserWithPassword(username,password);
        std::cout<<"Barrier Received: "<< BarrierString<<std::endl;
        return !BarrierString.empty();
    }
}