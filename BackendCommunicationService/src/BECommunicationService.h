//---------------------------------------------------------
// Created by Moritz Herzog on 13.12.23.
//---------------------------------------------------------
//---------------------------------------------------------
// Constants, Definitions, Pragmas
//---------------------------------------------------------
#pragma once
//---------------------------------------------------------
// External Classes
//---------------------------------------------------------
#include <vector>
#include <string>
#include <memory>
#include <exception>
#include <boost/uuid/uuid.hpp>
//---------------------------------------------------------
// Internal Classes
//---------------------------------------------------------
#include "cpp_backend_communication_global.h"
#include "entities/TwinRequest.h"
//---------------------------------------------------------
// Forwarding
//---------------------------------------------------------
namespace SysMLv2 {
    namespace REST {
        class IEntity;
        class Commit;
        class CommitRequest;
        class Branch;
        class Project;
        class ProjectRequest;
        class DigitalTwin;
    }

    namespace API {
        class SysMLAPIImplementation;
    }
}

namespace KerML {
    namespace Entities {
        class Element;
    }
}

namespace BACKEND_COMMUNICATION {
    /**
     * Abstracts the communication to an interface for the programmer. Allows the communication with the backend.
     * Hereby a connection to the server is opened. With this connection the REST protocol is generated and sent.
     * REST API Specification is here <a href="https://github.com/Systems-Modeling/SysML-v2-API-Services">https://github.com/Systems-Modeling/SysML-v2-API-Services</a>
     * @author Moritz Herzog <herzogm@rptu.de>
     * @version 1.0
     */
    class CPSBACKENDCOMMUNICATION_EXPORT CommunicationService {
    public:
        /**
         * Constructor allows for the settings for the connection to the AGILA Backend.
         * Port 443 results in "https://address/", port 80 in "http://address/" and every other port in
         * "http://address:port/".
         * @param serverAddress Address of the Server. The server address can be an IP address or a url.
         * @param port Port of the AGILA Backend server.
         * @param serverFolder Folder of the Backend. It is only stored and NOT appended to the base url, because the
         *        client settings still default to the legacy value "/agila-server/", which would break the
         *        connection to the current server.
         */
        CommunicationService(std::string serverAddress, unsigned int port, std::string serverFolder);
        /**
        * Constructor allows for the settings for the connection to the AGILA Backend.
        * @param serverAddress Address of the Server. The server address can be an IP address or a url. If it contains
        *        no scheme ("://"), "http://" is prepended.
        * @param serverFolder Folder where the Backend is directed to. It is appended to the address.
        */
        CommunicationService(std::string serverAddress, std::string serverFolder);

        /**
         * D-Tor. Defined in the source file, because the API implementation is only forward declared here.
         */
        virtual ~CommunicationService();

        CommunicationService(const CommunicationService&) = delete;
        CommunicationService& operator=(const CommunicationService&) = delete;

        /**
         * Creates a connection to the REST endpoint of the given AGILA Backend server,  to get all Elements of a Commit and Project, distinguished by its uuid.
         * @param commitId UUID of the commit
         * @param projectId UUID of the project
         * @return An std::vector of the Elements
         * @throws SysMLv2::API::EXCEPTIONS::HTTPException on a non 2xx answer of the server.
         * @throws SysMLv2::API::EXCEPTIONS::ConnectionError if the server is not reachable.
         * @see uuid_t
         * @see std::vector
         * @see ENTITIES::Element
         */
        std::vector<std::shared_ptr<KerML::Entities::Element>> getAllElements(boost::uuids::uuid commitId, boost::uuids::uuid projectId);

        /**
         * Creates a connection to the REST endpoint of the given AGILA Backend sever, to get all saved projects within the instance of the Backend
         * @return An std::vector of the Projects
         * @throws SysMLv2::API::EXCEPTIONS::HTTPException on a non 2xx answer of the server.
         * @throws SysMLv2::API::EXCEPTIONS::ConnectionError if the server is not reachable.
         */
        std::vector<std::shared_ptr<SysMLv2::REST::Project>> getAllProjects();

        /**
         * Creates online a project with the given Properties.
         * @param projectName The name of the Project.
         * @param projectDescription
         * @param defaultBranchName
         * @return The created project.
         * @throws SysMLv2::API::EXCEPTIONS::HTTPException on a non 2xx answer of the server.
         * @throws SysMLv2::API::EXCEPTIONS::ConnectionError if the server is not reachable.
         */
        std::shared_ptr<SysMLv2::REST::Project> postProject(std::string projectName, std::string projectDescription, std::string defaultBranchName);

        /**
         * Downloads the Digital Twin data with its ID and the project id.
         * @param digitalTwinId UUID ("@id") of the digital twin
         * @param projectId UUID ("@id") of the project.
         * @return The digital twins metadata, nullptr if the project has no twin with this id. The server offers no
         *         endpoint for a single twin, therefore the list of the project is downloaded and filtered.
         * @throws SysMLv2::API::EXCEPTIONS::HTTPException on a non 2xx answer of the server.
         * @throws SysMLv2::API::EXCEPTIONS::ConnectionError if the server is not reachable.
         * @throws std::runtime_error if the answer of the server is not a JSON array.
         */
        std::shared_ptr<SysMLv2::REST::DigitalTwin> getDigitalTwinWithID(boost::uuids::uuid digitalTwinId, boost::uuids::uuid projectId);

        /**
         * Downloads all Digital Twins for a specific project.
         * @param projectId the ID of the given project.
         * @return A vector of all digital twins.
         * @throws SysMLv2::API::EXCEPTIONS::HTTPException on a non 2xx answer of the server.
         * @throws SysMLv2::API::EXCEPTIONS::ConnectionError if the server is not reachable.
         * @throws std::runtime_error if the answer of the server is not a JSON array.
         */
        std::vector<std::shared_ptr<SysMLv2::REST::DigitalTwin>> getAllDigitalTwinsForProjectWithId(boost::uuids::uuid projectId);

        /**
         * Download all branches for a project, with the projects id.
         * @param projectId UUID ("@id") of the project.
         * @return All Branches of the Project.
         * @throws SysMLv2::API::EXCEPTIONS::HTTPException on a non 2xx answer of the server.
         * @throws SysMLv2::API::EXCEPTIONS::ConnectionError if the server is not reachable.
         */
        std::vector<std::shared_ptr<SysMLv2::REST::Branch>> getAllBranchesForProjectWithID(boost::uuids::uuid projectId);

        /**
         * Download the commits with its commit id and project id identifying the specific commit.
         * @param projectId UUID ("@id") of the project.
         * @param commitId UUID ("@id") of the commit.
         * @return The complete Commit
         * @throws SysMLv2::API::EXCEPTIONS::HTTPException on a non 2xx answer of the server.
         * @throws SysMLv2::API::EXCEPTIONS::ConnectionError if the server is not reachable.
         * @see ENTITIES::Commit
         */
        std::shared_ptr<SysMLv2::REST::Commit> getCommitWithId(boost::uuids::uuid projectId, boost::uuids::uuid commitId);

        /**
         * Posts a commit to the given project.
         * @param projectId UUID ("@id") of the project.
         * @param commit The commit request.
         * @return The created commit.
         * @throws SysMLv2::API::EXCEPTIONS::HTTPException on a non 2xx answer of the server.
         * @throws SysMLv2::API::EXCEPTIONS::ConnectionError if the server is not reachable.
         */
        std::shared_ptr<SysMLv2::REST::Commit> postCommitWithId(boost::uuids::uuid projectId, std::shared_ptr<SysMLv2::REST::CommitRequest> commit);


        /**
         * Downloads all the elements of a specific commit.
         * @param projectId UUID ("@id") of the project.
         * @param commitId UUID ("@id") of the commit.
         * @return All elements of the specific commit.
         * @throws SysMLv2::API::EXCEPTIONS::HTTPException on a non 2xx answer of the server.
         * @throws SysMLv2::API::EXCEPTIONS::ConnectionError if the server is not reachable.
         * @see ENTITIES::Element
         */
        std::vector<std::shared_ptr<KerML::Entities::Element>> getAllElementsOfCommit(boost::uuids::uuid projectId, boost::uuids::uuid commitId);

        /**
         * Sets and checks internally the user, that the server is connected to.
         * @param username The username or email of the user to login.
         * @param password The password of the user to login.
         * @return true on success. Failures are reported by exceptions, false is never returned.
         * @throws std::invalid_argument if the credentials are wrong (HTTP 401).
         * @throws std::runtime_error if the login is throttled (HTTP 429), the message contains the seconds to wait.
         * @throws SysMLv2::API::EXCEPTIONS::HTTPException on any other non 2xx answer of the server.
         * @throws SysMLv2::API::EXCEPTIONS::ConnectionError if the server is not reachable.
         * @throws std::runtime_error if the answer of the server is malformed.
         */
        bool setUserForLoginInBackend(std::string username, std::string password);

        /**
         * Creates a digital twin in the given project.
         * @param digitalTwin The twin request.
         * @param projectId UUID ("@id") of the project.
         * @return The created digital twin.
         * @throws SysMLv2::API::EXCEPTIONS::HTTPException on a non 2xx answer of the server.
         * @throws SysMLv2::API::EXCEPTIONS::ConnectionError if the server is not reachable.
         * @throws std::runtime_error if the answer of the server is not a JSON object.
         */
        std::shared_ptr<SysMLv2::REST::DigitalTwin> postDigitalTwin(std::shared_ptr<SysMLv2::REST::TwinRequest> digitalTwin, boost::uuids::uuid projectId);

        /**
         * Creates a short, user facing English message for an exception thrown by this class or the library.
         * Distinguishes HTTP 401 (session expired), 403 (password change required / no permission), 404, 409, 429 and
         * connection errors; all other exceptions result in their what().
         * @param exception The caught exception.
         * @return The message that can be shown to the user.
         */
        static std::string describeBackendError(const std::exception& exception);

    private:
        std::string REST_Protocol = "http://";
        std::string Entry_URI = "";
        std::string ServerAddress = "localhost";
        unsigned int Port = 8080;

        std::string BarrierString = "";

        std::unique_ptr<SysMLv2::API::SysMLAPIImplementation> APIImplementation;
    };
}
