//
// Created by Moritz Herzog on 17.01.24.
//

#pragma once
#include <string>
#include <cstdint>
#include <memory>
#include <functional>
#include <thread>
#include "Services/MqttClientService.h"
#include "cpp_physical_twin_communication_global.h"


namespace PHYSICAL_TWIN_COMMUNICATION {
    /**
     * @author Moritz Herzog
     * Creates the MQTT Communication Service for the Server or the Client.
     */
    class CPPPHYSICALTWINCOMMUNICATION_EXPORT CommunicationService {
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
         * Destructor
         */
        virtual  ~CommunicationService();

        void startThreads();

        void addObservationCallbackForTopic(std::string topic, std::function<void(std::string)> callback);

        void publishMQTTMessage(std::string topic, std::string content);

        void joinThreads();

        MqttClientService* getClientService();

    private:
        std::thread ClientThread;
        std::thread ServerThread;

        MqttClientService* ClientService;

        uint16_t MqttPort;
    };
}

