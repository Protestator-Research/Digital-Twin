//
// Created by Moritz Herzog on 22.07.25.
//

#include <nlohmann/json.hpp>
#include <sysmlv2/rest/entities/JSONEntities.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>

#include "DigitalTwin.h"

namespace SysMLv2::REST{
    DigitalTwin::DigitalTwin(std::string jsonString) : Record(jsonString) {
        try {
            nlohmann::json parsedJson = nlohmann::json::parse(jsonString);

            auto connectedModels = parsedJson[JSON_CONNECTED_MODELS].get<std::vector<std::string>>();
            for(auto model : connectedModels) {
                boost::uuids::uuid modelId = boost::uuids::string_generator()(model);
                ConnectedModels.push_back(modelId);
            }

            ParentProjectId = boost::uuids::string_generator()(parsedJson[JSON_PARENT_PROJECT].get<std::string>());


            CommitId = boost::uuids::string_generator()(parsedJson[JSON_COMMIT_ID].get<std::string>());

        }catch(...){}
    }

    DigitalTwin::DigitalTwin(std::string name, std::vector<boost::uuids::uuid> connectedElements,
	    boost::uuids::uuid commitId) : Record(boost::uuids::random_generator()(),name,"")
    {
        CommitId = commitId;
        ConnectedModels = connectedElements;
    }

    std::string DigitalTwin::serializeToJson() {
        nlohmann::json json = nlohmann::json::parse(Record::serializeToJson());
        json.erase(JSON_ID_ENTITY);
        json.erase(JSON_TYPE_ENTITY);
        json.erase(JSON_ALIAS_ENTITY);
        json[JSON_COMMIT_ID] = boost::uuids::to_string(CommitId);
        std::string connectedElementsString = "[\r\n";
        for (size_t i = 0; i < ConnectedModels.size(); i++)
        {
            connectedElementsString += "\""+boost::uuids::to_string(ConnectedModels[i])+"\"";
            if (i != ConnectedModels.size() - 1)
                connectedElementsString += ",\r\n";
        }
        connectedElementsString += "\r\n]";
        json["connectedElements"] = nlohmann::json::parse(connectedElementsString);
        return json.dump(JSON_INTENT);
    }

    bool DigitalTwin::operator==(const DigitalTwin &other) {
        return Record::operator==(other);
    }

    boost::uuids::uuid DigitalTwin::commitId() const {
        return CommitId;
    }

    boost::uuids::uuid DigitalTwin::parentProjectId() const {
        return ParentProjectId;
    }

    std::vector<boost::uuids::uuid> DigitalTwin::getConnectedModels() {
        return ConnectedModels;
    }
} // SysMLv2::REST