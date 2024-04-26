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
//---------------------------------------------------------
// Internal Classes
//---------------------------------------------------------

//---------------------------------------------------------
// Forwarding
//---------------------------------------------------------
namespace SysMLv2::Entities {
    class IEntity;
}

namespace BACKEND_COMMUNICATION {
    /**
     * Abstracts the communication to an interface for the programmer. Allows the communication with the backend.
     * Hereby a connection to the server is opened. With this connection the REST protocol is generated and sent.
     * REST API Specification is here <a href="https://github.com/Systems-Modeling/SysML-v2-API-Services">https://github.com/Systems-Modeling/SysML-v2-API-Services</a>
     * @author Moritz Herzog <herzogm@rptu.de>
     * @version 1.0
     */
    class CommunicationService {
    public:
        /**
         * Constructor allows for the settings for the connection to the AGILA Backend.
         * @param serverAddress Address of the Server. The server address can be an IP address or a url.
         * @param port Port of the AGILA Backend server.
         */
        CommunicationService(std::string serverAddress, unsigned int port);
        /**
        * Constructor allows for the settings for the connection to the AGILA Backend.
        * @param serverAddress Address of the Server. The server address can be an IP address or a url.
        */
        explicit CommunicationService(std::string serverAddress);

        /**
         * D-Tor
         */
        virtual ~CommunicationService() = default;

        /**
         * Creates a connection to the REST endpoint of the given AGILA Backend server,  to get all Elements of a Commit and Project, distinguished by its uuid.
         * @param commitId UUID of the commit
         * @param projectId UUID of the project
         * @return An std::vector of the Elements
         * @see uuid_t
         * @see std::vector
         * @see ENTITIES::Element
         */
//        std::vector<ENTITIES::Element> getAllElements(uuid_t commitId, uuid_t projectId);

        /**
         * Creates a connection to the REST endpoint of the given AGILA Backend sever, to get all saved projects within the instance of the Backend
         * @return An std::vector of the Projects
         */
        std::vector<SysMLv2::Entities::IEntity*> getAllProjects();

        /**
         * Downloads the Digital Twin data with its ID and the project id.
         * @param digitalTwinId UUID ("@id") of the digital twin
         * @param projectId UUID ("@id") of the project.
         * @return The digital twins metadata.
         */
//        ENTITIES::DigitalTwin getDigitalTwinWithID(uuid_t digitalTwinId, uuid_t projectId);

        /**
         * Download all branches for a project, with the projects id.
         * @param projectId UUID ("@id") of the project.
         * @return All Branches of the Project.
         */
//        std::vector<ENTITIES::Branch> getAllBranchesForProjectWithID(uuid_t projectId);

        /**
         * Download the commits with its commit id and project id identifying the specific commit.
         * @param projectId UUID ("@id") of the project.
         * @param commitId UUID ("@id") of the project
         * @return The complete Commit
         * @see ENTITIES::Commit
         */
//        ENTITIES::Commit getCommitWithId(uuid_t projectId, uuid_t commitId);

        /**
         * Sets and checks internally the user, that the server is connected to.
         * @param username The username or email of the user to login.
         * @param password The password of the user to login.
         * @return
         */
        bool setUserForLoginInBackend(std::string username, std::string password);

    private:
        std::string REST_PROTOCOL = "http://";
        const std::string ENTRY_URI = "/agila-server/";
        std::string ServerAddress = "localhost";
        unsigned int Port = 8080;

        std::string BarrierString = "";
    };
}
