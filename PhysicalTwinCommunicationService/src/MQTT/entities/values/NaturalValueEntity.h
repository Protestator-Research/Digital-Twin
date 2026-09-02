//
// Created by herzog on 18.08.26.
//

#pragma once
#include "ValueEntity.h"

namespace DigitalTwin::Communication
{
    class NaturalValueEntity : public ValueEntity
    {
    public:
        NaturalValueEntity() = delete;
        explicit NaturalValueEntity(unsigned int value);
        explicit NaturalValueEntity(std::string jsonString);
        virtual ~NaturalValueEntity() = default;

        unsigned int getValue();

        std::string getJson() const override;
        std::string getType() const override;

    private:
        unsigned int Value;
    };
}


