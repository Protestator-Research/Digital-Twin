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
#include <memory>
//---------------------------------------------------------
// Internal Classes
//---------------------------------------------------------
#include "Record.h"
//---------------------------------------------------------
// Forwarding
//---------------------------------------------------------
namespace SysMLv2::Entities {
    class DataVersion;
    class DataIdentity;
}


namespace SysMLv2::Entities {
    /**
     * Represents the object of the Project entity.
     * @class Project
     * @author Moritz Herzog <herzogm@rptu.de>
     * @version 1.0
     */
    class Project  : public Record {
    public:
        /**
         * Constructor
         */
        Project() = delete;

        /**
         * Copy constructor
         * @param other
         */
        Project(Project &other);

        /**
         *
         * @param JsonString
         */
        explicit Project(std::string JsonString);

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
                DataIdentity* defaultBranchId,
                std::list<DataIdentity*> branchesIdList,
                std::list<DataIdentity*> commitIdList,
                std::list<DataIdentity*> headIdList);

        /**
         * Destructor
         */
        ~Project() override;

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
        [[nodiscard]] std::time_t getCreationDate() const;

        /**
         *
         * @param identity
         */
        void setDefaultBranch(DataIdentity* identity);

        /**
         *
         * @return
         */
        DataIdentity* getDefaultBranch() const;

        /**
         *
         * @return
         */
        std::list<DataIdentity*> getBranches() const;

        /**
         *
         * @param branchId
         */
        void appendBranch(DataIdentity* branchId);

        /**
         *
         * @return
         */
        std::list<DataIdentity*> getCommitsList() const;

        /**
         *
         * @param commitId
         */
        void appendCommit(DataIdentity* commitId);

        /**
         *
         * @return
         */
        std::list<DataIdentity*> getHeadsIdList() const;

        /**
         *
         * @param headId
         */
        void appendHead(DataIdentity* headId);
    private:
        std::chrono::system_clock::time_point CreationDate;

        DataIdentity* DefaultBranch;
        std::list<DataIdentity*> BranchesList;
        std::list<DataIdentity*> CommitsList;
        std::list<DataIdentity*> HeadIdList;

    };
}

#endif //DIGITALTWIN_PROJECT_H
