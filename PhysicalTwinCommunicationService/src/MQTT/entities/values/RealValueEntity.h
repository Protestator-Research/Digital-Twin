//
// Created by herzog on 18.08.26.
//

#pragma once

#include "ValueEntity.h"

namespace DigitalTwin::Communication
{
    class CPPPHYSICALTWINCOMMUNICATION_EXPORT RealValueEntity : public ValueEntity
    {
    public:
        RealValueEntity() = delete;
        RealValueEntity(double value);
        RealValueEntity(std::string jsonString);
        virtual ~RealValueEntity() = default;

        double getValue();

        std::string getJson() const override;
        std::string getType() const override;

    private:
        double Value;
    };
}

