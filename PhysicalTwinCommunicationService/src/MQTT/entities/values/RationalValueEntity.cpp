//
// Created by herzog on 18.08.26.
//

#include "RationalValueEntity.h"
#include "../JsonEntities.h"
#include "BaseFuctions/StringExtention.hpp"

#include <nlohmann/json.hpp>

#include <stdexcept>


namespace
{
    /**
     * Parses one side of "n/d": an optionally signed integer, surrounded by optional whitespace, nothing else.
     */
    long parseRationalPart(const std::string& text, const std::string& valueString)
    {
        const auto invalid = [&valueString]() {
            return std::invalid_argument("invalid rational \"" + valueString + "\", expected \"numerator/denominator\"");
        };
        const auto first = text.find_first_not_of(" \t");
        if (first == std::string::npos)
            throw invalid();
        const auto last = text.find_last_not_of(" \t");
        const std::string trimmed = text.substr(first, last - first + 1);
        std::size_t consumed = 0;
        long value = 0;
        try {
            value = std::stol(trimmed, &consumed);
        } catch (const std::exception&) {
            throw invalid();   // not a number or out of the range of long
        }
        if (consumed != trimmed.size())
            throw invalid();   // trailing characters like "2abc" or "1.5"
        return value;
    }

    /**
     * Parses "n/d" with exactly one '/' and a non-zero denominator, like the Rust server (payload/values.rs).
     */
    boost::rational<long> parseRational(const std::string& valueString)
    {
        const auto slash = valueString.find('/');
        if (slash == std::string::npos || valueString.find('/', slash + 1) != std::string::npos)
            throw std::invalid_argument("invalid rational \"" + valueString + "\", expected exactly one '/'");
        const long numerator = parseRationalPart(valueString.substr(0, slash), valueString);
        const long denominator = parseRationalPart(valueString.substr(slash + 1), valueString);
        if (denominator == 0)
            throw std::invalid_argument("invalid rational \"" + valueString + "\", the denominator is 0");
        return boost::rational<long>(numerator, denominator);
    }
}

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
        Value = parseRational(valueString);
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
