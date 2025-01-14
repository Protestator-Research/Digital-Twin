//
// Created by Moritz Herzog on 17.01.24.
//

#include "PTCommunicationService.h"

#include "Services/MqttBrokerService.h"

#include <thread>
#include <chrono>

#ifdef WIN32
#include <windows.h>
#endif

namespace PHYSICAL_TWIN_COMMUNICATION {

    CommunicationService::CommunicationService(std::string mqttPort) {
        MqttPort=std::stoi(mqttPort);
        ClientService = new MqttClientService("localhost",std::to_string(MqttPort));
    }

    void CommunicationService::startThreads() {
        try {
            ServerThread = std::thread([this]{
                MQTTBrokerService::runBroker(MqttPort);
            });

            ClientThread = std::thread([this] {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                ClientService->connectClientStartCommunication();
            });
        }
        catch(std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    void CommunicationService::addObservationCallbackForTopic(std::string topic, std::function<void(std::string)> callback, std::string initValue) {
        ClientService->addCallbackFunction(topic,callback, initValue);
    }

    void CommunicationService::publishMQTTMessage(std::string topic, std::string content) {
        ClientService->sendValueToServer(topic, content);
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