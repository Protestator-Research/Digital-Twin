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
#include <sysmlv2/rest/entities/IEntity.h>
#include <sysmlv2/rest/entities/Project.h>
#include <sysmlv2/rest/entities/Commit.h>
#include <sysmlv2/rest/entities/Branch.h>
#include <kerml/root/elements/Element.h>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>
#include <sysmlv2/service/online/SysMLAPIImplementation.h>

//---------------------------------------------------------
// Internal Classes
//---------------------------------------------------------
#include "BECommunicationService.h"


namespace BACKEND_COMMUNICATION {
    CommunicationService::CommunicationService(std::string serverAddress, unsigned int port, std::string serverFolder) :
            Entry_URI(serverFolder),
            ServerAddress(serverAddress),
            Port(port)
    {
        if(Port==443)
            REST_Protocol="https://";

        APIImplementation = new SysMLv2::API::SysMLAPIImplementation(REST_Protocol + ServerAddress + ":" + std::to_string(Port) + "/");
    }

    CommunicationService::CommunicationService(std::string serverAddress, std::string serverFolder) {
        ServerAddress = std::move(serverAddress);
        Entry_URI = std::move(serverFolder);
    }

    std::vector<std::shared_ptr<KerML::Entities::Element>> CommunicationService::getAllElements(boost::uuids::uuid commitId, boost::uuids::uuid projectId) {
        auto entities = APIImplementation->getAllElementsFromCommit(boost::lexical_cast<std::string>(projectId),boost::lexical_cast<std::string>(commitId), BarrierString);
        std::vector<std::shared_ptr<KerML::Entities::Element>> elements;

        for(auto entitiy : entities)
            elements.push_back(dynamic_pointer_cast<KerML::Entities::Element>(entitiy));

        return elements;
    }

    std::vector<std::shared_ptr<SysMLv2::REST::Project>> CommunicationService::getAllProjects() {
        auto projects = APIImplementation->getAllProjects(BarrierString);
        std::vector<std::shared_ptr<SysMLv2::REST::Project>> returnValue;

        for(auto oldProject : projects)
            returnValue.push_back(dynamic_pointer_cast<SysMLv2::REST::Project>(oldProject));

        return returnValue;
    }

    std::shared_ptr<SysMLv2::REST::DigitalTwin> CommunicationService::getDigitalTwinWithID(boost::uuids::uuid , boost::uuids::uuid ) {
        return nullptr;
    }

    std::vector<std::shared_ptr<SysMLv2::REST::DigitalTwin>> CommunicationService::getAllDigitalTwinsForProjectWithId(boost::uuids::uuid ) {
//        auto twins = APIImplementation->getAllDigitalTwinsForProject(boost::lexical_cast<std::string>(projectId),BarrierString);
        std::vector<std::shared_ptr<SysMLv2::REST::DigitalTwin>> returnValue;

//        for(auto oldTwin : twins)
//            returnValue.push_back(dynamic_cast<SysMLv2::Entities::DigitalTwin*>(oldTwin));

        return returnValue;
    }


    std::shared_ptr<SysMLv2::REST::Commit> CommunicationService::getCommitWithId(boost::uuids::uuid , boost::uuids::uuid ) {
        return nullptr;
    }

    std::shared_ptr<SysMLv2::REST::Commit> CommunicationService::postCommitWithId(boost::uuids::uuid projectId,
                                                                                      std::shared_ptr<SysMLv2::REST::Commit> commit)
    {
        auto commi = APIImplementation->postCommit(boost::lexical_cast<std::string>(projectId), commit, BarrierString);

        std::shared_ptr<SysMLv2::REST::Commit> returnValue = dynamic_pointer_cast<SysMLv2::REST::Commit>(commi);
        return returnValue;
    }


    bool CommunicationService::setUserForLoginInBackend(std::string username, std::string password) {
        BarrierString = APIImplementation->loginUserWithPassword(username,password);
        return !BarrierString.empty();
    }

    std::shared_ptr<SysMLv2::REST::DigitalTwin> CommunicationService::postDigitalTwin(std::shared_ptr<SysMLv2::REST::DigitalTwin> ,
	    boost::uuids::uuid )
    {
//        auto digitalT = APIImplementation->postDigitalTwin(boost::lexical_cast<std::string>(projectId), digitalTwin, BarrierString);

        std::shared_ptr<SysMLv2::REST::DigitalTwin> returnValue = nullptr;
        return returnValue;
    }

    std::vector<std::shared_ptr<SysMLv2::REST::Branch>> CommunicationService::getAllBranchesForProjectWithID(boost::uuids::uuid projectId) {
        auto elements = APIImplementation->getAllBranchesFroProject(boost::lexical_cast<std::string>(projectId), BarrierString);

        std::vector<std::shared_ptr<SysMLv2::REST::Branch>> returnValue;

        for (auto elem : elements)
            returnValue.push_back(dynamic_pointer_cast<SysMLv2::REST::Branch>(elem));

        return returnValue;
    }

    std::vector<std::shared_ptr<KerML::Entities::Element >>
    CommunicationService::getAllElementsOfCommit(boost::uuids::uuid projectId, boost::uuids::uuid commitId) {
        auto elements = APIImplementation->getAllElementsFromCommit(boost::lexical_cast<std::string>(projectId),boost::lexical_cast<std::string>(commitId), BarrierString);
        std::vector<std::shared_ptr<KerML::Entities::Element>> returnValue;

        for(auto elem : elements)
            returnValue.push_back(dynamic_pointer_cast<KerML::Entities::Element>(elem));

        return returnValue;
    }

    std::shared_ptr<SysMLv2::REST::Project>
    CommunicationService::postProject(std::string projectName, std::string projectDescription, std::string defaultBranchName) {
        std::shared_ptr<SysMLv2::REST::Project> project = std::make_shared<SysMLv2::REST::Project>(projectName,projectDescription, defaultBranchName);
        project = std::dynamic_pointer_cast<SysMLv2::REST::Project>(APIImplementation->postProject(project, BarrierString));
        return project;
    }


}