//
// Created by herzog on 18.08.26.
//

#pragma once

#include "ValueEntity.h"

namespace DigitalTwin::Communication
{
    class PositiveValueEntity : public ValueEntity
    {
    public:
        PositiveValueEntity() = delete;
        PositiveValueEntity(unsigned int value);
        PositiveValueEntity(std::string jsonString);
        virtual ~PositiveValueEntity() = default;

        unsigned int getValue();

        std::string getJson() const override;
        std::string getType() const override;

    private:
        unsigned int Value;
    };
}
