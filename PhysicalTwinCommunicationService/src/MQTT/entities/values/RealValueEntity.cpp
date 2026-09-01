//
// Created by herzog on 18.08.26.
//

#include "RealValueEntity.h"
#include "../JsonEntities.hpp"
#include "BaseFuctions/StringExtention.hpp"

#include <nlohmann/json.hpp>

namespace DigitalTwin::Communication
{
    RealValueEntity::RealValueEntity(double value)
    {
        Value = value;
    }

    RealValueEntity::RealValueEntity(std::string jsonString)
    {
        nlohmann::json json = nlohmann::json::parse(jsonString);
        Value = json[JsonEntities::VALUE_ENTITY];
        Timepoint = CPSBASELIB::STD_EXTENTION::StringExtention::timepointFromString(json[JsonEntities::TIME_POINT_ENTITY]);
    }

    double RealValueEntity::getValue()
    {
        return Value;
    }

    std::string RealValueEntity::getJson() const
    {
        nlohmann::json json;
        json[JsonEntities::TIME_POINT_ENTITY] = CPSBASELIB::STD_EXTENTION::StringExtention::timepointToString(Timepoint);
        json[JsonEntities::VALUE_ENTITY] = Value;
        return json.dump();
    }

    std::string RealValueEntity::getType() const
    {
        return "RealValueEntity";
    }
}
