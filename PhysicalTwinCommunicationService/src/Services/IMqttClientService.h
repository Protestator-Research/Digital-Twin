//
// Created by Moritz Herzog on 18.03.26.
//

#pragma once

#include <string>
#include "../cpp_physical_twin_communication_global.h"

namespace DigitalTwin::Communication {
    class CPPPHYSICALTWINCOMMUNICATION_EXPORT IMqttClientService {
    public:
        virtual void publish(std::string topic, std::string content) = 0;

    };
}
