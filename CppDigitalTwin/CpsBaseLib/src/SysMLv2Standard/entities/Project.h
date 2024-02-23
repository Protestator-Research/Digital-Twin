//
// Created by Moritz Herzog on 20.02.24.
//
//---------------------------------------------------------
// Constants, Definitions, Pragmas
//---------------------------------------------------------
#ifndef DIGITALTWIN_PROJECT_H
#define DIGITALTWIN_PROJECT_H
//---------------------------------------------------------
// External Classes
//---------------------------------------------------------
#include <boost/uuid/uuid.hpp>
#include <ctime>
#include <string>
//---------------------------------------------------------
// Internal Classes
//---------------------------------------------------------
#include "Record.h"
#include "Identity.h"
//---------------------------------------------------------
// Forwarding
//---------------------------------------------------------

namespace SysMLv2::Entities {
    /**
     * Represents the object of the
     * @class Project
     * @author Moritz Herzog <herzogm@rptu.de>
     * @version 1.0
     */
    class Project  : public Record {
    public:
        /**
         * Constructor
         */
        Project() = default;

        /**
         * Copy constructor
         * @param other
         */
        Project(Project &other);

        /**
         *
         * @param JsonString
         */
        Project(std::string JsonString);

        /**
         *
         * @param id
         * @param alias
         * @param name
         * @param description
         * @param creationDate
         * @param defaultBranchId
         * @param branchesIdList
         * @param commitIdList
         * @param headIdList
         */
        Project(boost::uuids::uuid id,
                std::list<std::string> alias,
                std::string name,
                std::string description,
                std::time_t creationDate,
                Identity defaultBranchId,
                std::list<Identity> branchesIdList,
                std::list<Identity> commitIdList,
                std::list<Identity> headIdList);

        /**
         * Destructor
         */
        virtual ~Project();

        /**
         *
         * @param other
         * @return
         */
        Project &operator=(const Project &other);

        /**
         * Allows the checking if two Projects are equal.
         * @param other
         * @return
         */
        bool operator==(const Project &other);

        std::string serializeToJson() override;

        /**
         *
         * @param creationDate
         */
        void setCreationDate(std::time_t creationDate);

        /**
         *
         * @return
         */
        std::time_t getCreationDate() const;

        void setDefaultBranch(Identity identity);

        Identity getDefaultBranch() const;


    private:
        std::chrono::system_clock::time_point CreationDate;

        Identity DefaultBranch;
        std::list<Identity> BranchesList;
        std::list<Identity> CommitsList;
        std::list<Identity> HeadIdList;

    };
}

#endif //DIGITALTWIN_PROJECT_H
