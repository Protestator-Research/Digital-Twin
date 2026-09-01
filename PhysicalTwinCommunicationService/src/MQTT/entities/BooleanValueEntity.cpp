//
// Created by herzog on 18.08.26.
//

#include "BooleanValueEntity.h"

#include <nlohmann/json.hpp>

#include "JsonEntities.hpp"
#include "BaseFuctions/ListExtentions.h"
#include "BaseFuctions/StringExtention.hpp"

namespace DigitalTwin::Communication
{
    BooleanValueEntity::BooleanValueEntity(bool value)
    {
        Value = value;
    }

    BooleanValueEntity::BooleanValueEntity(std::string jsonString)
    {
        nlohmann::json json = nlohmann::json::parse(jsonString);
        Value= json[JsonEntities::VALUE_ENTITY];
        Timepoint = CPSBASELIB::STD_EXTENTION::StringExtention::timepointFromString(json[JsonEntities::TIME_POINT_ENTITY]);
    }

    bool BooleanValueEntity::getValue()
    {
        return Value;
    }

    std::string BooleanValueEntity::getJson() const
    {
        nlohmann::json json;
        json[JsonEntities::VALUE_ENTITY] = Value;
        json[JsonEntities::TIME_POINT_ENTITY] = CPSBASELIB::STD_EXTENTION::StringExtention::timepointToString(Timepoint);
        return json.dump();
    }

    std::string BooleanValueEntity::getType() const
    {
        return "BooleanValue";
    }
}
