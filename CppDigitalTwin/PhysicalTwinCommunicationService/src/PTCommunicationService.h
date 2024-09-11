//
// Created by Moritz Herzog on 17.01.24.
//

#pragma once
#include <string>
#include <cstdint>
#include <memory>
#include <functional>
#include "Services/MqttClientService.h"


namespace PHYSICAL_TWIN_COMMUNICATION {
    /**
     * @author Moritz Herzog
     * Creates the MQTT Communication Service for the Server or the Client.
     */
    class CommunicationService {
    public:
        /**
         * Is Deleted
         */
        CommunicationService() = delete;
        /**
         * Generates a Server instance with the given port as the listening port.
         * @param mqttPort The listening Port of the
         */
        explicit CommunicationService(std::string mqttPort);

        /**
         * Generates a client instance of the MQTT Interface.
         * @param address Address of the server.
         * @param mqttPort Port of the Server
         */
        CommunicationService(std::string address, std::string mqttPort);

        /**
         * Destructor
         */
        virtual  ~CommunicationService() = default;

        void startThreads();

        void addObservationCallbackForTopic(std::string topic, std::function<void(std::string)> callback);

        void publishMQTTMessage(std::string topic, std::string content);
    private:


        std::unique_ptr<MqttClientService> ClientService;

        uint16_t MqttPort;
    };
}

