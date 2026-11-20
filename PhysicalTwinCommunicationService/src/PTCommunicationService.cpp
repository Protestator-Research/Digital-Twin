//
// Created by Moritz Herzog on 17.01.24.
//

#include "PTCommunicationService.h"


#include <thread>
#include <chrono>
#include <iostream>

#if defined(_WIN32) || defined(WIN32)
#include <windows.h>
#endif

namespace DigitalTwin::Communication {

    CommunicationService::CommunicationService(std::string mqttPort, MqttProtocolVersion version,
                                               std::string username, std::string password) {
        MqttPort=std::stoi(mqttPort);
        ClientService = new MqttClientService("localhost",std::to_string(MqttPort),"", version,
                                              std::move(username), std::move(password));
    }

    void CommunicationService::startThreads() {
        try {
            ClientThread = std::thread([this] {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                ClientService->start();
            });
        }
        catch(std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    void CommunicationService::addObservationCallbackForTopic([[maybe_unused]] std::string topic,[[maybe_unused]] std::function<void(std::string)> callback,[[maybe_unused]] std::string initValue) {
        //ClientService->addCallbackFunction(topic,callback, initValue);
    }

    void CommunicationService::publishMQTTMessage(std::string topic, std::string content) {
        ClientService->publish(topic, content);
    }

    void CommunicationService::joinThreads() {
        ServerThread.join();
    }

    CommunicationService::~CommunicationService() {
        delete ClientService;
    }

    MqttClientService *CommunicationService::getClientService() {
        return ClientService;
    }
}