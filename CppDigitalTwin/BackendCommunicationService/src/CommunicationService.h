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
//---------------------------------------------------------
// Internal Classes
//---------------------------------------------------------
#include "Entities/Element.hpp"
#include "Entities/Project.hpp"
#include "Entities/DigitalTwin.hpp"
#include "Entities/Branch.hpp"
#include "Entities/Commit.hpp"
//---------------------------------------------------------
// Forwarding
//---------------------------------------------------------
namespace boost::uuids {
    class uuid;
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
         * C-Tor
         */
        CommunicationService();
        /**
         * D-Tor
         */
        virtual ~CommunicationService() = default;

        /**
         *
         * @param commitId
         * @param projectId
         * @return
         */
        std::vector<ENTITIES::Element> getAllElements(boost::uuids::uuid commitId, boost::uuids::uuid projectId);

        /**
         *
         * @return
         */
        std::vector<ENTITIES::Project> getAllProjects();

        /**
         *
         * @param digitalTwinId
         * @param projectId
         * @return
         */
        ENTITIES::DigitalTwin getDigitalTwinWithID(boost::uuids::uuid digitalTwinId, boost::uuids::uuid projectId);

        /**
         *
         * @param projectId
         * @return
         */
        std::vector<ENTITIES::Branch> getAllBranchesForProjectWithID(boost::uuids::uuid projectId);

        /**
         *
         * @param projectId
         * @param commitId
         * @return
         */
        ENTITIES::Commit getCommitWithId(boost::uuids::uuid projectId, boost::uuids::uuid commitId);

    private:

    };
}
