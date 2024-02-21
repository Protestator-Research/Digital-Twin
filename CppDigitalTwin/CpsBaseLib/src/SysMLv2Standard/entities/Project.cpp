//
// Created by Moritz Herzog on 20.02.24.
//

#include "Project.h"

namespace SysMLv2::Entities {
    Project::Project() : Record() {

    }

    Project::Project(Project &other) : Record(other) {

    }

    Project::Project(std::string JsonString) : Record() {
        JsonString;
    }

    Project::Project(boost::uuids::uuid id, std::list<std::string> alias, std::string name, std::string description,
                     std::time_t creationDate, boost::uuids::uuid defaultBranchId,
                     std::list<boost::uuids::uuid> branchesIdList, std::list<boost::uuids::uuid> commitIdList,
                     std::list<boost::uuids::uuid> headIdList) : Record(id,alias,name,description) {
        creationDate;
        defaultBranchId;
        branchesIdList;
        commitIdList;
        headIdList;
    }

    Project::Project(boost::uuids::uuid id, std::list<std::string> alias, std::string name, std::string description,
                     std::time_t creationDate, boost::uuids::uuid defaultBranchId, std::list<Identity> branchesIdList,
                     std::list<Identity> commitIdList, std::list<Identity> headIdList) : Record(id,alias,name,description) {
        creationDate;
        description;
        defaultBranchId;
        branchesIdList;
        commitIdList;
        headIdList;
    }

    Project &Project::operator=(const Project &other) {
        if(this == &other)
            return *this;

        Record::Id = other.Id;
        Record::Alias = other.Alias;
        Record::Name = other.Name;
        Record::Description = other.Description;

        CreationDate = other.CreationDate;

        BranchesList.clear();
        for(Identity element : other.BranchesList) {
            BranchesList.emplace_back(element);
        }

        CommitsList.clear();
        for(Identity element : other.CommitsList) {
            CommitsList.emplace_back(element);
        }

        HeadIdList.clear();
        for(Identity element : other.HeadIdList) {
            HeadIdList.emplace_back(element);
        }

        return *this;
    }

    bool Project::operator==(const Project &other) {
        if((*dynamic_cast<Record*>(this))==(other)){
            if(CreationDate!=other.CreationDate)
                return false;

            if(DefaultBranch.getId() != other.DefaultBranch.getId())
                return false;

            return false;
        }
        else {
            return false;
        }
    }

    std::string Project::serializeToJson() {
        return std::string();
    }

    Project::~Project() {
        BranchesList.clear();
        CommitsList.clear();
        HeadIdList.clear();
    }

}