//
// Created by herzog on 18.08.26.
//

#include "RationalValueEntity.h"
#include "JsonEntities.hpp"
#include "BaseFuctions/StringExtention.hpp"

#include <nlohmann/json.hpp>




namespace DigitalTwin::Communication
{
    RationalValueEntity::RationalValueEntity(long numerator, long denominator)
    {
        Value = boost::rational<long>(numerator, denominator);
    }

    RationalValueEntity::RationalValueEntity(const boost::rational<long>& value)
    {
        Value = value;
    }

    RationalValueEntity::RationalValueEntity(std::string jsonString)
    {
        nlohmann::json json = nlohmann::json::parse(jsonString);
        std::string valueString = json[JsonEntities::VALUE_ENTITY];
        const auto valueStringSplit = CPSBASELIB::STD_EXTENTION::StringExtention::splitString(valueString,'/');
        Value = boost::rational<long>(std::stol(valueStringSplit[0]),std::stol(valueStringSplit[1]));
        Timepoint = CPSBASELIB::STD_EXTENTION::StringExtention::timepointFromString(json[JsonEntities::TIME_POINT_ENTITY]);
    }

    std::string RationalValueEntity::getJson() const
    {
        nlohmann::json json;
        json[JsonEntities::TIME_POINT_ENTITY] = CPSBASELIB::STD_EXTENTION::StringExtention::timepointToString(Timepoint);
        std::string valueString = std::to_string(Value.numerator()) + "/" + std::to_string(Value.denominator());
        json[JsonEntities::VALUE_ENTITY] = valueString;
        return json.dump();
    }

    std::string RationalValueEntity::getType() const
    {
        return "RationalValueEntity";
    }

    boost::rational<long> RationalValueEntity::getValue() const
    {
        return Value;
    }
}
