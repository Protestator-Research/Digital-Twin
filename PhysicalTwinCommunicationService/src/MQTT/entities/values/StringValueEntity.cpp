//
// Created by herzog on 18.08.26.
//

#include <nlohmann/json.hpp>

#include "StringValueEntity.h"
#include <BaseFuctions/StringExtention.hpp>
#include "../JsonEntities.h"

namespace DigitalTwin::Communication
{
    StringValueEntity::StringValueEntity(std::string value) : Value(std::move(value))
    {
    }

    StringValueEntity StringValueEntity::fromJson(std::string jsonString)
    {
        nlohmann::json json = nlohmann::json::parse(jsonString);
        StringValueEntity entity(json[JsonEntities::VALUE_ENTITY].get<std::string>());
        entity.Timepoint = CPSBASELIB::STD_EXTENTION::StringExtention::timepointFromString(json[JsonEntities::TIME_POINT_ENTITY]);
        return entity;
    }

    std::string StringValueEntity::getValue() const
    {
        return Value;
    }

    std::string StringValueEntity::getJson() const
    {
        nlohmann::json json;
        json[JsonEntities::TIME_POINT_ENTITY] = CPSBASELIB::STD_EXTENTION::StringExtention::timepointToString(Timepoint);
        json[JsonEntities::VALUE_ENTITY] = Value;
        // Invalid UTF-8 in the value is replaced instead of throwing (type_error 316).
        return json.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace);
    }

    std::string StringValueEntity::getType() const
    {
        return "StringValue";
    }
}
