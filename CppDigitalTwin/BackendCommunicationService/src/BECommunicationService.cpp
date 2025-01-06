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
#include <SysMLv2Standard/entities/Element.h>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

//---------------------------------------------------------
// Internal Classes
//---------------------------------------------------------
#include "BECommunicationService.h"
#include "APIImplementations/SysMLAPIImplementation.h"


namespace BACKEND_COMMUNICATION {
    CommunicationService::CommunicationService(std::string serverAddress, unsigned int port, std::string serverFolder) :
            Entry_URI(serverFolder),
            ServerAddress(serverAddress),
            Port(port)
    {
        if(Port==443)
            REST_Protocol="https://";

        APIImplementation = new SysMLAPIImplementation(REST_Protocol + ServerAddress + ":" + std::to_string(Port) + Entry_URI);
    }

    CommunicationService::CommunicationService(std::string serverAddress, std::string serverFolder) {
        ServerAddress = std::move(serverAddress);
        Entry_URI = std::move(serverFolder);
    }

    std::vector<SysMLv2::Entities::Element*> CommunicationService::getAllElements(boost::uuids::uuid commitId, boost::uuids::uuid projectId) {
        auto entities = APIImplementation->getAllElementsFromCommit(boost::lexical_cast<std::string>(projectId),boost::lexical_cast<std::string>(commitId), BarrierString);
        std::vector<SysMLv2::Entities::Element*> elements;

        for(auto entitiy : entities)
            elements.push_back(dynamic_cast<SysMLv2::Entities::Element*>(entitiy));

        return elements;
    }

    std::vector<SysMLv2::Entities::Project*> CommunicationService::getAllProjects() {
        auto projects = APIImplementation->getAllProjects(BarrierString);
        std::vector<SysMLv2::Entities::Project*> returnValue;

        for(auto oldProject : projects)
            returnValue.push_back(dynamic_cast<SysMLv2::Entities::Project*>(oldProject));

        return returnValue;
    }

    SysMLv2::Entities::DigitalTwin* CommunicationService::getDigitalTwinWithID(boost::uuids::uuid , boost::uuids::uuid ) {
        return new SysMLv2::Entities::DigitalTwin("null");
    }

    std::vector<SysMLv2::Entities::DigitalTwin*> CommunicationService::getAllDigitalTwinsForProjectWithId(boost::uuids::uuid projectId) {
        auto twins = APIImplementation->getAllDigitalTwinsForProject(boost::lexical_cast<std::string>(projectId),BarrierString);
        std::vector<SysMLv2::Entities::DigitalTwin*> returnValue;

        for(auto oldTwin : twins)
            returnValue.push_back(dynamic_cast<SysMLv2::Entities::DigitalTwin*>(oldTwin));

        return returnValue;
    }


    SysMLv2::Entities::Commit* CommunicationService::getCommitWithId(boost::uuids::uuid , boost::uuids::uuid ) {
        return nullptr;
    }



    bool CommunicationService::setUserForLoginInBackend(std::string username, std::string password) {
        BarrierString = APIImplementation->loginUserWithPassword(username,password);
        return !BarrierString.empty();
    }

    std::vector<SysMLv2::Entities::Branch*> CommunicationService::getAllBranchesForProjectWithID(boost::uuids::uuid projectId) {
        auto elements = APIImplementation->getAllBrachesFroProject(boost::lexical_cast<std::string>(projectId), BarrierString);

        std::vector<SysMLv2::Entities::Branch*> returnValue;

        for (auto elem : elements)
            returnValue.push_back(dynamic_cast<SysMLv2::Entities::Branch*>(elem));

        return returnValue;
    }

    std::vector<SysMLv2::Entities::Element *>
    CommunicationService::getAllElementsOfCommit(boost::uuids::uuid projectId, boost::uuids::uuid commitId) {
        auto elements = APIImplementation->getAllElementsFromCommit(boost::lexical_cast<std::string>(projectId),boost::lexical_cast<std::string>(commitId), BarrierString);
        std::vector<SysMLv2::Entities::Element*> returnValue;

        for(auto elem : elements)
            returnValue.push_back(dynamic_cast<SysMLv2::Entities::Element*>(elem));

        return returnValue;
    }
}