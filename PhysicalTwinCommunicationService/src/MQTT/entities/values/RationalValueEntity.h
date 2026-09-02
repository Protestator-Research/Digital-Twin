//
// Created by herzog on 18.08.26.
//

#pragma once

#include "ValueEntity.h"

#include <boost/rational.hpp>

namespace DigitalTwin::Communication
{
    class RationalValueEntity : public ValueEntity
    {
    public:
        RationalValueEntity() = delete;
        RationalValueEntity(long numerator, long denominator);
        RationalValueEntity(const boost::rational<long>& value);
        RationalValueEntity(std::string jsonString);

        boost::rational<long> getValue() const;

        std::string getJson() const override;
        std::string getType() const override;

    private:
        boost::rational<long> Value;
    };
}
