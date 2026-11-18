//
// Created by herzog on 18.08.26.
//

#pragma once

#include "ValueEntity.h"

namespace DigitalTwin::Communication
{
    class CPPPHYSICALTWINCOMMUNICATION_EXPORT IntegerValueEntity : public ValueEntity
    {
    public:
        IntegerValueEntity() = delete;
        IntegerValueEntity(int value);
        IntegerValueEntity(std::string jsonString);
        virtual ~IntegerValueEntity() = default;

        int getValue();

        std::string getJson()  const override;
        std::string getType() const override;

    private:
        int Value;
    };
} // PHYSICAL_TWIN_COMMUNICATION

