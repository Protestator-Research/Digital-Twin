//
// Created by Moritz Herzog on 29.02.24.
//

#include "SysMLv2Deserializer.h"
#include "entities/JSONEntities.h"
#include "entities/Project.h"
#include <nlohmann/json.hpp>

namespace SysMLv2 {
    SysMLv2::Entities::IEntity *SysMLv2Deserializer::deserializeJsonString(std::string inputValue) {
        nlohmann::json json = nlohmann::json::parse(inputValue);

        if(json[Entities::JSON_TYPE_ENTITY]==Entities::PROJECT_TYPE)
            return new Entities::Project(inputValue);




        return nullptr;
    }
} // SysMLv2