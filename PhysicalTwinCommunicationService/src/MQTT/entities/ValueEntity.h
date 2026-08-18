//
// Created by Moritz Herzog on 14.01.25.
//

#pragma once

#include <chrono>


namespace DigitalTwin::Communication {
    class ValueEntity {
    public:
        ValueEntity() = default;
        virtual ~ValueEntity() = default;

        std::chrono::time_point<std::chrono::system_clock> getTimepoint();

        virtual std::string getJson() const = 0;

        virtual std::string getType() const = 0;

    protected:
        std::chrono::time_point<std::chrono::system_clock> Timepoint = std::chrono::system_clock::now();

    };
}
