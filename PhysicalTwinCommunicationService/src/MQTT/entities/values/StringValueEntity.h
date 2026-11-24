//
// Created by herzog on 18.08.26.
//

#pragma once

#include <string>

#include "ValueEntity.h"

namespace DigitalTwin::Communication
{
    class CPPPHYSICALTWINCOMMUNICATION_EXPORT StringValueEntity : public ValueEntity
    {
    public:
        StringValueEntity() = delete;
        /**
         * Creates the entity from the string value itself.
         * Since the value and a JSON string are both std::string, the JSON variant is
         * not a constructor but the static factory fromJson().
         */
        explicit StringValueEntity(std::string value);
        virtual ~StringValueEntity() = default;

        /**
         * Creates the entity from a JSON string (see getJson()).
         */
        static StringValueEntity fromJson(std::string jsonString);

        std::string getValue() const;

        std::string getJson() const override;
        std::string getType() const override;

    private:
        std::string Value;
    };
}
