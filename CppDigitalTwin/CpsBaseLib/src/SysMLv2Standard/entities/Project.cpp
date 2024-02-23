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

        std::istringstream stringStream(parsedJson[JSON_CREATION_ENTITY].get<std::string>());
        stringStream.imbue(std::locale("de_DE.utf-8"));
        stringStream >> std::get_time(CreationDate,"%Y-%m-%dT%TZ");

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
        nlohmann::json json = nlohmann::json::parse(Record::serializeToJson());

        json[JSON_DEFAULT_BRANCH_ENTITY] = DefaultBranch.serializeToJson();
        std::ostringstream oss;
        oss << std::put_time(CreationDate,"%Y-%m-%dT%TZ");
        json[JSON_CREATION_ENTITY] = oss.str();

        if(!BranchesList.empty()) {
            auto branchesListArray = nlohmann::json::array();

            for(auto element : BranchesList)
                branchesListArray.push_back(element.serializeToJson());

            json[JSON_BRANCHES_LIST] = branchesListArray;
        }

        if(!CommitsList.empty()) {
            auto commitsListArray = nlohmann::json::array();

            for(auto element : CommitsList)
                commitsListArray.push_back(element.serializeToJson());

            json[JSON_COMMIT_LIST] = commitsListArray;
        }

        if(!HeadIdList.empty()) {
            auto headsListArray = nlohmann::json::array();

            for(auto element : HeadIdList)
                headsListArray.push_back(element.serializeToJson());

            json[JSON_HEAD_ID_LIST] = headsListArray;
        }

        return json.dump(JSON_INTENT);
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