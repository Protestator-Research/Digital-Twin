//
// Created by herzog on 18.08.26.
//

#include "NaturalValueEntity.h"
#include "JsonEntities.hpp"
#include "BaseFuctions/StringExtention.hpp"

#include <nlohmann/json.hpp>



namespace DigitalTwin::Communication
{
    NaturalValueEntity::NaturalValueEntity(unsigned int value)
    {
        Value = value;
    }

    NaturalValueEntity::NaturalValueEntity(std::string jsonString)
    {
        nlohmann::json json = nlohmann::json::parse(jsonString);
        Value = json[JsonEntities::VALUE_ENTITY];
        Timepoint = CPSBASELIB::STD_EXTENTION::StringExtention::timepointFromString(json[JsonEntities::TIME_POINT_ENTITY]);
    }

    unsigned int NaturalValueEntity::getValue()
    {
        return Value;
    }

    std::string NaturalValueEntity::getJson() const
    {
        nlohmann::json json;
        json[JsonEntities::TIME_POINT_ENTITY] = CPSBASELIB::STD_EXTENTION::StringExtention::timepointToString(Timepoint);
        json[JsonEntities::VALUE_ENTITY] = Value;
        return json.dump();
    }

    std::string NaturalValueEntity::getType() const
    {
        return "NaturalValueEntity";
    }
}
