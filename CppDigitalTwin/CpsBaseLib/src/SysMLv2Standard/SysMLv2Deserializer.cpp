//
// Created by Moritz Herzog on 29.02.24.
//

#include "SysMLv2Deserializer.h"
#include "entities/JSONEntities.h"
#include "entities/Project.h"
#include "entities/DataIdentity.h"
#include "entities/Branch.h"
#include "entities/Tag.h"
#include "entities/Query.h"
#include "entities/DigitalTwin.h"
#include "../BaseFuctions/StringExtention.hpp"
#include <nlohmann/json.hpp>
#include <vector>

namespace SysMLv2 {
    SysMLv2::Entities::IEntity *SysMLv2Deserializer::deserializeJsonString(std::string inputValue) {
        nlohmann::json json = nlohmann::json::parse(inputValue);

        std::string type = CPSBASELIB::STD_EXTENTION::StringExtention::toLower(
                json[Entities::JSON_TYPE_ENTITY].get<std::string>());

        if(type==Entities::PROJECT_TYPE)
            return new Entities::Project(inputValue);

        if(type==Entities::DATA_IDENTITY_TYPE)
            return new Entities::DataIdentity(inputValue);

        if(type==Entities::BRANCH_TYPE)
            return new Entities::Branch(inputValue);

        if(type==Entities::TAG_TYPE)
            return new Entities::Tag(inputValue);

        if(type==Entities::QUERY_TYPE)
            return new Entities::Query(inputValue);

        if(type==Entities::DIGITAL_TWIN_TYPE)
            return new Entities::DigitalTwin(inputValue);

        return nullptr;
    }

    std::vector<SysMLv2::Entities::IEntity*> SysMLv2Deserializer::deserializeJsonArray(std::string inputValue) {
        nlohmann::json json = nlohmann::json::parse(inputValue);
        std::vector<nlohmann::json> arrayValues = json.get<std::vector<nlohmann::json>>();
        std::vector<SysMLv2::Entities::IEntity*> returnValues;
        returnValues.reserve(arrayValues.size());
        for(const nlohmann::json& elem : arrayValues) {
            returnValues.emplace_back(SysMLv2Deserializer::deserializeJsonString(elem.dump()));
        }
        return returnValues;
    }
} // SysMLv2