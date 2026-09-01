//
// Created by herzog on 18.08.26.
//

#include "PositiveValueEntity.h"
#include "../JsonEntities.hpp"

#include <nlohmann/json.hpp>

#include "BaseFuctions/StringExtention.hpp"

namespace DigitalTwin::Communication
{
    PositiveValueEntity::PositiveValueEntity(unsigned int value)
    {
        Value = value;
    }

    PositiveValueEntity::PositiveValueEntity(std::string jsonString)
    {
        nlohmann::json json = nlohmann::json::parse(jsonString);
        Value = json[JsonEntities::VALUE_ENTITY];
        Timepoint = CPSBASELIB::STD_EXTENTION::StringExtention::timepointFromString(json[JsonEntities::TIME_POINT_ENTITY]);
    }

    unsigned int PositiveValueEntity::getValue()
    {
        return Value;
    }

    std::string PositiveValueEntity::getJson() const
    {
        nlohmann::json json;
        json[JsonEntities::TIME_POINT_ENTITY] = CPSBASELIB::STD_EXTENTION::StringExtention::timepointToString(Timepoint);
        json[JsonEntities::VALUE_ENTITY] = Value;
        return json.dump();
    }

    std::string PositiveValueEntity::getType() const
    {
        return "PositiveValueEntity";
    }
}
