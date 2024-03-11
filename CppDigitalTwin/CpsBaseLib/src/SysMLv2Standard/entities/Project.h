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
#include <chrono>
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
    class Commit;
    class CommitReference;
    class Branch;
    class ProjectUsage;
    class Tag;
    class Query;
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
         * Generates a Project object from a JSON string. This will be used for the JSON interface of the Parser.
         * @param JsonString
         */
        explicit Project(std::string JsonString);

        virtual ~Project();

        Branch* getDefaultBranch();

    private:
        DataVersion* DataVersion;


        std::chrono::system_clock::time_point Created;

        std::vector<Commit*> Commits;
        std::vector<CommitReference*> CommitReferences;
        std::vector<Branch*> Branches;
        Branch* DefaultBranch;
        std::vector<Tag*> Tags;
        std::vector<ProjectUsage*> ProjectUsages;
        std::vector<Query*> Querries;
    };
}

#endif //DIGITALTWIN_PROJECT_H
