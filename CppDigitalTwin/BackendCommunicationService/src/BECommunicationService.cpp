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
#include <SysMLv2Standard/entities/IEntity.h>
#include <SysMLv2Standard/entities/Project.h>
#include <SysMLv2Standard/entities/Commit.h>
#include <SysMLv2Standard/entities/Branch.h>
#include <SysMLv2Standard/entities/DigitalTwin.h>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

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
        if(Port==443)
            REST_PROTOCOL="https://";

        if(!SysMLAPIImplementation::connectToServer(REST_PROTOCOL + ServerAddress + ":" + std::to_string(Port) + ENTRY_URI))
            throw EXCEPTIONS::ConnectionError(EXCEPTIONS::CONNECTION_ERROR_TYPE::COULD_NOT_CONNECT);
    }

    CommunicationService::CommunicationService(std::string serverAddress) {
        ServerAddress = std::move(serverAddress);
    }

    std::vector<SysMLv2::Entities::IEntity*> CommunicationService::getAllElements(boost::uuids::uuid , boost::uuids::uuid ) {

        return std::vector<SysMLv2::Entities::IEntity*>();
    }

    std::vector<SysMLv2::Entities::Project*> CommunicationService::getAllProjects() {
        auto projects = SysMLAPIImplementation::getAllProjects(BarrierString);
        std::vector<SysMLv2::Entities::Project*> returnValue;

        for(auto oldProject : projects)
            returnValue.push_back(dynamic_cast<SysMLv2::Entities::Project*>(oldProject));

        return returnValue;
    }

    SysMLv2::Entities::DigitalTwin* CommunicationService::getDigitalTwinWithID(boost::uuids::uuid , boost::uuids::uuid ) {
        return new SysMLv2::Entities::DigitalTwin("null");
    }

    std::vector<SysMLv2::Entities::DigitalTwin*> CommunicationService::getAllDigitalTwinsForProjectWithId(boost::uuids::uuid projectId) {
        auto twins = SysMLAPIImplementation::getAllDigitalTwinsForProject(boost::lexical_cast<std::string>(projectId),BarrierString);
        std::vector<SysMLv2::Entities::DigitalTwin*> returnValue;

        for(auto oldTwin : twins)
            returnValue.push_back(dynamic_cast<SysMLv2::Entities::DigitalTwin*>(oldTwin));

        return returnValue;
    }


    SysMLv2::Entities::Commit* CommunicationService::getCommitWithId(boost::uuids::uuid , boost::uuids::uuid ) {
        return nullptr;
    }

    bool CommunicationService::setUserForLoginInBackend(std::string username, std::string password) {
        BarrierString = SysMLAPIImplementation::loginUserWithPassword(username,password);
        return !BarrierString.empty();
    }

    std::vector<SysMLv2::Entities::Branch*> CommunicationService::getAllBranchesForProjectWithID(boost::uuids::uuid ) {
        return std::vector<SysMLv2::Entities::Branch *>();
    }
}