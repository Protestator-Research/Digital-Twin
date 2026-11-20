//
// Created by Moritz Herzog on 18.03.26.
//

#pragma once

#include <functional>
#include <string>
#include "../cpp_physical_twin_communication_global.h"

namespace DigitalTwin::Communication {
    /**
     * The MQTT protocol version a client speaks. "3.1" in the settings means MQTT 3.1.1 (protocol level 4), the
     * old MQTT 3.1 (MQIsdp) is not supported.
     */
    enum class MqttProtocolVersion {
        V3_1_1,
        V5
    };

    class CPPPHYSICALTWINCOMMUNICATION_EXPORT IMqttClientService {
    public:
        virtual ~IMqttClientService() = default;

        virtual void publish(std::string topic, std::string content) = 0;

        /**
         * Subscribes to a topic. The callback is called for every message of exactly this topic, from a thread
         * of the MQTT library.
         */
        virtual void subscribe(std::string topic, std::function<void(std::string topic, std::string payload)> callback) = 0;
    };
}
