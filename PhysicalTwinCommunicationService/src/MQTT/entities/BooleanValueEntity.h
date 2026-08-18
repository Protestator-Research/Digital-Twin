//
// Created by herzog on 18.08.26.
//

#pragma once
#include "ValueEntity.h"

namespace DigitalTwin::Communication
{
    class BooleanValueEntity : public ValueEntity
    {
    public:
        BooleanValueEntity() = delete;
        BooleanValueEntity(bool value);
        BooleanValueEntity(std::string jsonString);

        bool getValue();

        std::string getJson() const override;
        std::string getType() const override;

    private:
        bool Value;
    };
}

