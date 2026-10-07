//
// Created by herzog on 18.08.26.
//

#include "ComplexValueEntity.h"
#include "../JsonEntities.h"
#include "BaseFuctions/StringExtention.hpp"

#include <nlohmann/json.hpp>

namespace DigitalTwin::Communication
{
    ComplexValueEntity::ComplexValueEntity(double real, double imaginary)
    {
        Value = std::complex<double>(real, imaginary);
    }

    ComplexValueEntity::ComplexValueEntity(std::complex<double> value)
    {
        Value = value;
    }

    ComplexValueEntity::ComplexValueEntity(std::string jsonString)
    {
        nlohmann::json json = nlohmann::json::parse(jsonString);
        Value = std::complex<double>(json[JsonEntities::REAL_VALUE_ENTITY], json[JsonEntities::IMAGINARY_VALUE_ENTITY]);
        Timepoint = CPSBASELIB::STD_EXTENTION::StringExtention::timepointFromString(json[JsonEntities::TIME_POINT_ENTITY]);
    }

    std::complex<double> ComplexValueEntity::getValue() const
    {
        return Value;
    }

    std::string ComplexValueEntity::getJson() const
    {
        nlohmann::json json;
        json[JsonEntities::REAL_VALUE_ENTITY] = Value.real();
        json[JsonEntities::IMAGINARY_VALUE_ENTITY] = Value.imag();
        json[JsonEntities::TIME_POINT_ENTITY] = CPSBASELIB::STD_EXTENTION::StringExtention::timepointToString(Timepoint);
        return json.dump();
    }

    std::string ComplexValueEntity::getType() const
    {
        return "ComplexValueEntity";
    }
}
