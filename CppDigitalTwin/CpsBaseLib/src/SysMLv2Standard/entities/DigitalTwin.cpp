//
// Created by Moritz Herzog on 23.02.24.
//

#include "DigitalTwin.h"
#include "JSONEntities.h"
#include "DataIdentity.h"

#include <nlohmann/json.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>


namespace SysMLv2::Entities {
    DigitalTwin::DigitalTwin(std::string jsonString) : Record(jsonString) {
        try {
            nlohmann::json parsedJson = nlohmann::json::parse(jsonString);

            auto connectedModels = parsedJson[JSON_CONNECTED_MODELS].get<std::vector<std::string>>();
            for(auto model : connectedModels) {
                boost::uuids::uuid modelId = boost::uuids::string_generator()(model);
                ConnectedModels.push_back(new DataIdentity(modelId));
            }

            ParentProjectId = new DataIdentity( boost::uuids::string_generator()(parsedJson[JSON_PARENT_PROJECT].get<std::string>()));

            if(parsedJson[JSON_BRANCH_ID].get<std::string>()!="null")
                BranchId=new DataIdentity(boost::uuids::string_generator()(parsedJson[JSON_BRANCH_ID].get<std::string>()));

            CommitId = new DataIdentity(boost::uuids::string_generator()(parsedJson[JSON_COMMIT_ID].get<std::string>()));

        }catch(...){}
    }

    DigitalTwin::~DigitalTwin() {
        for(auto elem : ConnectedModels)
            delete elem;

        ConnectedModels.clear();

        delete ParentProjectId;

        if(BranchId != nullptr)
            delete BranchId;

        delete CommitId;
    }

    std::string DigitalTwin::serializeToJson() {
        return Record::serializeToJson();
    }

    bool DigitalTwin::operator==(const DigitalTwin &other) {
        return Record::operator==(other);
    }

}