//
// Created by Moritz Herzog on 07.08.24.
//

#include "DigitalTwinEntity.h"
#include "JsonEntities.hpp"

#include <nlohmann/json.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace PHYSICAL_TWIN_COMMUNICATION {

    DigitalTwinEntity::DigitalTwinEntity(boost::uuids::uuid digitalTwinId, boost::uuids::uuid projectId) :
    DigitalTwinId(digitalTwinId),ProjectId(projectId)
    {    }

    DigitalTwinEntity::DigitalTwinEntity(std::string json) {
        nlohmann::json json_doc = nlohmann::json::parse(json);
        DigitalTwinId = boost::uuids::string_generator()(json_doc[DIGITAL_TWIN_ID_ENTITY].get<std::string>());
        ProjectId = boost::uuids::string_generator()(json_doc[PROJECT_ID_ENTITY].get<std::string>());
    }

    std::string DigitalTwinEntity::serialize() {
        nlohmann::json json_doc;
        json_doc[DIGITAL_TWIN_ID_ENTITY] = boost::uuids::to_string(DigitalTwinId);
        json_doc[PROJECT_ID_ENTITY] = boost::uuids::to_string(ProjectId);
        return json_doc.dump();
    }

    boost::uuids::uuid DigitalTwinEntity::digitalTwinId() const {
        return DigitalTwinId;
    }

    boost::uuids::uuid DigitalTwinEntity::projectId() const {
        return ProjectId;
    }
}