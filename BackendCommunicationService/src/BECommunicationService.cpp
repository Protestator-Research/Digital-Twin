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
#include <stdexcept>
#include <string>
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

#include <nlohmann/json.hpp>
#include <sysmlv2/service/online/HttpException.h>
#include <sysmlv2/service/online/ConnectionError.h>
#include <sysmlv2/rest/entities/CommitRequest.h>
#include <sysmlv2/rest/entities/ProjectRequest.h>

#include "entities/DigitalTwin.h"


namespace BACKEND_COMMUNICATION {
    namespace {
        /**
         * Joins the parts of an url with exactly one '/' between them and a trailing '/'.
         */
        std::string joinUrl(const std::string& base, const std::string& folder) {
            std::string result = base;
            while (!result.empty() && result.back() == '/')
                result.pop_back();
            std::size_t first = folder.find_first_not_of('/');
            if (first != std::string::npos) {
                std::string trimmed = folder.substr(first);
                while (!trimmed.empty() && trimmed.back() == '/')
                    trimmed.pop_back();
                result += "/" + trimmed;
            }
            return result + "/";
        }
    }

    CommunicationService::CommunicationService(std::string serverAddress, unsigned int port, std::string serverFolder) :
            Entry_URI(std::move(serverFolder)),
            ServerAddress(std::move(serverAddress)),
            Port(port)
    {
        if(Port==443)
            REST_Protocol="https://";

        // The folder is intentionally not part of the url, see the documentation in the header.
        if ((Port!=443)&&(Port!=80))
            APIImplementation = std::make_unique<SysMLv2::API::SysMLAPIImplementation>(REST_Protocol + ServerAddress + ":" + std::to_string(Port) + "/");
        else
            APIImplementation = std::make_unique<SysMLv2::API::SysMLAPIImplementation>(REST_Protocol + ServerAddress);

    }

    CommunicationService::CommunicationService(std::string serverAddress, std::string serverFolder) {
        ServerAddress = std::move(serverAddress);
        Entry_URI = std::move(serverFolder);

        std::string baseUrl = ServerAddress.find("://") == std::string::npos ? REST_Protocol + ServerAddress : ServerAddress;
        APIImplementation = std::make_unique<SysMLv2::API::SysMLAPIImplementation>(joinUrl(baseUrl, Entry_URI));
    }

    CommunicationService::~CommunicationService() = default;

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

    std::shared_ptr<SysMLv2::REST::DigitalTwin> CommunicationService::getDigitalTwinWithID(boost::uuids::uuid digitalTwinId, boost::uuids::uuid projectId) {
        // The server offers no endpoint for a single twin.
        for(auto& twin : getAllDigitalTwinsForProjectWithId(projectId))
            if(twin && twin->getId() == digitalTwinId)
                return twin;
        return nullptr;
    }

    std::vector<std::shared_ptr<SysMLv2::REST::DigitalTwin>> CommunicationService::getAllDigitalTwinsForProjectWithId(boost::uuids::uuid projectId) {
        auto twinString = APIImplementation->getCustomRequest("projects/" + boost::lexical_cast<std::string>(projectId) + "/twins",BarrierString);
        std::vector<std::shared_ptr<SysMLv2::REST::DigitalTwin>> returnValue;
        nlohmann::json json = nlohmann::json::parse(twinString, nullptr, false);
        if(!json.is_array())
            throw std::runtime_error("The answer of the server for the digital twins is not a JSON array.");
        returnValue.reserve(json.size());
        for(const nlohmann::json& elem : json) {
            returnValue.emplace_back(std::make_shared<SysMLv2::REST::DigitalTwin>(elem.dump()));
        }
        return returnValue;
    }


    std::shared_ptr<SysMLv2::REST::Commit> CommunicationService::getCommitWithId(boost::uuids::uuid projectId, boost::uuids::uuid commitId) {
        auto commit = APIImplementation->getCommit(boost::lexical_cast<std::string>(projectId), boost::lexical_cast<std::string>(commitId), BarrierString);
        return std::dynamic_pointer_cast<SysMLv2::REST::Commit>(commit);
    }

    std::shared_ptr<SysMLv2::REST::Commit> CommunicationService::postCommitWithId(boost::uuids::uuid projectId,
                                                                                      std::shared_ptr<SysMLv2::REST::CommitRequest> commit)
    {
        auto commi = APIImplementation->postCommit(boost::lexical_cast<std::string>(projectId), commit, BarrierString);

        std::shared_ptr<SysMLv2::REST::Commit> returnValue = dynamic_pointer_cast<SysMLv2::REST::Commit>(commi);
        return returnValue;
    }


    bool CommunicationService::setUserForLoginInBackend(std::string username, std::string password) {
        try {
            BarrierString = APIImplementation->loginUserWithPassword(username,password);
        } catch(const SysMLv2::API::EXCEPTIONS::HTTPException& ex) {
            if(ex.httpErrorCode() == 401)
                throw std::invalid_argument("Invalid username or password");
            if(ex.httpErrorCode() == 429) {
                std::string message = "Too many login attempts";
                if(ex.retryAfterSeconds() >= 0)
                    message += ", retry in " + std::to_string(ex.retryAfterSeconds()) + " s";
                throw std::runtime_error(message);
            }
            throw;
        }
        return true;
    }

    std::shared_ptr<SysMLv2::REST::DigitalTwin> CommunicationService::postDigitalTwin(std::shared_ptr<SysMLv2::REST::TwinRequest> digitalTwin,
	    boost::uuids::uuid projectId)
    {
        auto digitalT = APIImplementation->postCustomRequest("projects/" + boost::lexical_cast<std::string>(projectId) + "/twins", digitalTwin->serializeToJson(), BarrierString);

        nlohmann::json json = nlohmann::json::parse(digitalT, nullptr, false);
        if(!json.is_object())
            throw std::runtime_error("The answer of the server for the created digital twin is not a JSON object.");

        return std::make_shared<SysMLv2::REST::DigitalTwin>(json.dump());
    }

    std::string CommunicationService::describeBackendError(const std::exception& exception) {
        if(auto http = dynamic_cast<const SysMLv2::API::EXCEPTIONS::HTTPException*>(&exception)) {
            switch(http->httpErrorCode()) {
                case 401:
                    return "Your session has expired or you are not logged in. Please log in again.";
                case 403:
                    if(http->serverErrorCode() == "password_change_required")
                        return "You must change your password before you can continue. Please do so in the web interface.";
                    return "You do not have permission for this action.";
                case 404:
                    return "The requested resource was not found or you have no access to it.";
                case 409:
                    return "The request conflicts with the current state on the server.";
                case 429: {
                    std::string message = "Too many attempts.";
                    if(http->retryAfterSeconds() >= 0)
                        message += " Please retry in " + std::to_string(http->retryAfterSeconds()) + " s.";
                    else
                        message += " Please retry later.";
                    return message;
                }
                default:
                    return http->what();
            }
        }
        if(dynamic_cast<const SysMLv2::API::EXCEPTIONS::ConnectionError*>(&exception))
            return std::string("The server is not reachable: ") + exception.what();
        return exception.what();
    }

    std::vector<std::shared_ptr<SysMLv2::REST::Branch>> CommunicationService::getAllBranchesForProjectWithID(boost::uuids::uuid projectId) {
        auto elements = APIImplementation->getAllBranchesFroProject(boost::lexical_cast<std::string>(projectId), BarrierString);

        std::vector<std::shared_ptr<SysMLv2::REST::Branch>> returnValue;

        for (auto elem : elements)
            returnValue.push_back(dynamic_pointer_cast<SysMLv2::REST::Branch>(elem));

        return returnValue;
    }

    std::vector<std::shared_ptr<KerML::Entities::Element>>
    CommunicationService::getAllElementsOfCommit(boost::uuids::uuid projectId, boost::uuids::uuid commitId) {
        auto elements = APIImplementation->getAllElementsFromCommit(boost::lexical_cast<std::string>(projectId),boost::lexical_cast<std::string>(commitId), BarrierString);
        std::vector<std::shared_ptr<KerML::Entities::Element>> returnValue;

        for(auto elem : elements)
            returnValue.push_back(dynamic_pointer_cast<KerML::Entities::Element>(elem));

        return returnValue;
    }

    std::shared_ptr<SysMLv2::REST::Project>
    CommunicationService::postProject(std::string projectName, std::string projectDescription, std::string defaultBranchName) {
        auto projectRequest = std::make_shared<SysMLv2::REST::ProjectRequest>(projectName,projectDescription, defaultBranchName);
        auto project = std::dynamic_pointer_cast<SysMLv2::REST::Project>(APIImplementation->postProject(projectRequest, BarrierString));
        return project;
    }


}
