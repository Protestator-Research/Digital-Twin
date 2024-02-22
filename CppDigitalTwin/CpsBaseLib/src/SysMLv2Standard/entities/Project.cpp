//
// Created by Moritz Herzog on 20.02.24.
//

#include <nlohmann/json.hpp>
#include <vector>
#include <sstream>
#include "Project.h"
#include "JSONEntities.h"
#include "../../BaseFuctions/StringExtention.hpp"

namespace SysMLv2::Entities {
    Project::Project(Project &other) : Record(other) {
        CreationDate = other.CreationDate;

        DefaultBranch = other.DefaultBranch;
        BranchesList = other.BranchesList;
        CommitsList = other.CommitsList;
        HeadIdList = other.HeadIdList;
    }

    Project::Project(std::string JsonString) : Record(JsonString) {
        nlohmann::json parsedJson = nlohmann::json::parse(JsonString);

        auto splittedTime = CPSBASELIB::STD_EXTENTION::StringExtention::splitString(parsedJson[JSON_CREATION].get<std::string>(),'.')[0];
        std::istringstream stringStream(splittedTime);
        //stringStream.imbue(std::locale("de_DE.utf-8"));
        stringStream >> std::get_time(CreationDate,"%Y-%m-%dT%H:%M:%S");

        DefaultBranch = Identity(parsedJson[JSON_DEFAULT_BRANCH_ENTITY].dump());
    }

    Project::Project(boost::uuids::uuid id, std::list<std::string> alias, std::string name, std::string description,
                     std::time_t creationDate, Identity defaultBranchId, std::list<Identity> branchesIdList,
                     std::list<Identity> commitIdList, std::list<Identity> headIdList) : Record(id,alias,name,description) {
        CreationDate = std::localtime(&creationDate);
        Description = description;
        DefaultBranch = defaultBranchId;
        BranchesList = branchesIdList;
        CommitsList = commitIdList;
        HeadIdList = headIdList;
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
        delete CreationDate;

        BranchesList.clear();
        CommitsList.clear();
        HeadIdList.clear();
    }

    Identity Project::getDefaultBranch() const {
        return DefaultBranch;
    }

    void Project::setDefaultBranch(Identity identity) {
        DefaultBranch = identity;
    }

    void Project::setCreationDate(std::time_t creationDate) {
        CreationDate = std::gmtime(&creationDate);
    }

    std::time_t Project::getCreationDate() const{
        return std::mktime(CreationDate);
    }
}