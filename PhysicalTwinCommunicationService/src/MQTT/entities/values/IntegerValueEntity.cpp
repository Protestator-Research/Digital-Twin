//
// Created by herzog on 18.08.26.
//

#include <nlohmann/json.hpp>

#include "IntegerValueEntity.h"
#include <BaseFuctions/StringExtention.hpp>
#include "JsonEntities.h"

namespace DigitalTwin::Communication
{
    IntegerValueEntity::IntegerValueEntity(int value)
    {
        Value = value;
    }

    IntegerValueEntity::IntegerValueEntity(std::string jsonString)
    {
        nlohmann::json json = nlohmann::json::parse(jsonString);
        Value = json[JsonEntities::VALUE_ENTITY];
        Timepoint = CPSBASELIB::STD_EXTENTION::StringExtention::timepointFromString(json[JsonEntities::TIME_POINT_ENTITY]);
    }

    int IntegerValueEntity::getValue()
    {
        return Value;
    }

    std::string IntegerValueEntity::getJson() const
    {
        nlohmann::json json;
        json[JsonEntities::TIME_POINT_ENTITY] = CPSBASELIB::STD_EXTENTION::StringExtention::timepointToString(Timepoint);
        json[JsonEntities::VALUE_ENTITY] = Value;
        return json.dump();
    }

    std::string IntegerValueEntity::getType() const
    {
        return "IntegerValue";
    }
} // PHYSICAL_TWIN_COMMUNICATION