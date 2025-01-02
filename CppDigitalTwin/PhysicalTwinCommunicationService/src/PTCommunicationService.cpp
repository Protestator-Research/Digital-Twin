//
// Created by Moritz Herzog on 17.01.24.
//

#include "PTCommunicationService.h"

#include "Services/MqttBrokerService.h"

#include <thread>

#ifdef WIN32
#include <windows.h>
#endif

namespace PHYSICAL_TWIN_COMMUNICATION {

    CommunicationService::CommunicationService(std::string mqttPort) {
        MqttPort=std::stoi(mqttPort);
        ClientService = std::make_unique<MqttClientService>("localhost",mqttPort);
    }

    void CommunicationService::startThreads() {
        try {
            bool serverStarted = false;
            std::thread serverThread = std::thread([&]{
                MQTTBrokerService::runBroker(MqttPort,serverStarted);
            });

            sleep(1);
            ClientService->connectClientStartCommunication();

            serverThread.join();
        }
        catch(std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    void CommunicationService::addObservationCallbackForTopic(std::string topic, std::function<void(std::string)> callback) {
        ClientService->addCallbackFunction(topic,callback);
    }

    void CommunicationService::publishMQTTMessage(std::string topic, std::string content) {
        ClientService->sendValueToServer(topic, content);
    }
}