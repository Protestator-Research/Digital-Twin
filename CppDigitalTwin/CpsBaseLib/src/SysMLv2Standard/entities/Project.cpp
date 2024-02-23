//
// Created by Moritz Herzog on 20.02.24.
//

#include <nlohmann/json.hpp>
#include <vector>
#include <sstream>
#include <date/date.h>

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
        stringStream >> date::parse("%Y-%m-%dT%TZ",CreationDate);

        DefaultBranch = DataIdentity(parsedJson[JSON_DEFAULT_BRANCH_ENTITY].dump());
    }

    Project::Project(boost::uuids::uuid id, std::list<std::string> alias, std::string name, std::string description,
                     std::time_t creationDate, DataIdentity defaultBranchId, std::list<DataIdentity> branchesIdList,
                     std::list<DataIdentity> commitIdList, std::list<DataIdentity> headIdList) : Record(id,alias,name,description) {
        CreationDate = std::chrono::system_clock::from_time_t(creationDate);
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
        for(DataIdentity element : other.BranchesList) {
            BranchesList.emplace_back(element);
        }

        CommitsList.clear();
        for(DataIdentity element : other.CommitsList) {
            CommitsList.emplace_back(element);
        }

        HeadIdList.clear();
        for(DataIdentity element : other.HeadIdList) {
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

        json[JSON_DEFAULT_BRANCH_ENTITY] = nlohmann::json::parse(DefaultBranch.serializeToJson());
        std::ostringstream oss;
        using namespace date;
        date::to_stream(oss, "%Y-%m-%dT%TZ", CreationDate);
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
        BranchesList.clear();
        CommitsList.clear();
        HeadIdList.clear();
    }

    DataIdentity Project::getDefaultBranch() const {
        return DefaultBranch;
    }

    void Project::setDefaultBranch(DataIdentity identity) {
        DefaultBranch = identity;
    }

    void Project::setCreationDate(std::time_t creationDate) {
        CreationDate = std::chrono::system_clock::from_time_t(creationDate);
    }

    std::time_t Project::getCreationDate() const{
        return std::chrono::system_clock::to_time_t(CreationDate);
    }

    std::list<DataIdentity> Project::getBranches() const {
        return BranchesList;
    }

    void Project::appendBranch(DataIdentity branchId) {
        BranchesList.push_back(branchId);
    }

    std::list<DataIdentity> Project::getCommitsList() const {
        return CommitsList;
    }

    void Project::appendCommit(DataIdentity commitId) {
        CommitsList.push_back(commitId);
    }

    std::list<DataIdentity> Project::getHeadsIdList() const {
        return HeadIdList;
    }

    void Project::appendHead(DataIdentity headId) {
        HeadIdList.push_back(headId);
    }
}