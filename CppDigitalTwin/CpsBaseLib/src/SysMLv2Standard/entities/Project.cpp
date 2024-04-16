//
// Created by Moritz Herzog on 20.02.24.
//
//---------------------------------------------------------
// Constants, Definitions, Pragmas
//---------------------------------------------------------

//---------------------------------------------------------
// External Classes
//---------------------------------------------------------
#include <nlohmann/json.hpp>
#include <vector>
#include <sstream>
#include <date/date.h>
//---------------------------------------------------------
// Internal Classes
//---------------------------------------------------------
#include "Project.h"
#include "DataIdentity.h"
#include "DataVersion.h"
#include "CommitReference.h"
#include "Commit.h"
#include "JSONEntities.h"
#include "Branch.h"
#include "ProjectUsage.h"
#include "Tag.h"
#include "Query.h"
#include "../../BaseFuctions/StringExtention.hpp"


namespace SysMLv2::Entities {
    Project::Project(Project &other) : Record(other) {
        Created = other.Created;

        Commits = other.Commits;

        CommitReferences = other.CommitReferences;

        DefaultBranch = other.DefaultBranch;
        Branches = other.Branches;

        Tags = other.Tags;

        ProjectUsages = other.ProjectUsages;

        Querries = other.Querries;
        DataVersion = other.DataVersion;
    }

    Project::Project(std::string JsonString) : Record(JsonString) {

    }

    Project::~Project() {

        for(auto elem : Commits)
            delete elem;
        Commits.clear();

        for(auto elem : CommitReferences)
            delete elem;
        CommitReferences.clear();

        for(auto elem : Branches)
            delete elem;
        Branches.clear();

        if(DefaultBranch != nullptr)
            delete DefaultBranch;

        for(auto elem : Tags)
            delete elem;
        Tags.clear();

        for(auto elem : ProjectUsages)
            delete elem;
        ProjectUsages.clear();

        for(auto elem : Querries)
            delete elem;
        Querries.clear();
    }

    Branch *Project::getDefaultBranch() {
        return nullptr;
    }

    std::string Project::serializeToJson() {
//        return Record::serializeToJson();
        return std::string();
    }
}
