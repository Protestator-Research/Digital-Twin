//
// Created by Moritz Herzog on 14.01.25.
//

#include "ValueEntity.h"

namespace DigitalTwin::Communication
{
    std::chrono::time_point<std::chrono::system_clock> ValueEntity::getTimepoint()
    {
        return Timepoint;
    }
}
