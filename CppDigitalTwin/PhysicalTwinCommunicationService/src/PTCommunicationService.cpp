//
// Created by Moritz Herzog on 17.01.24.
//

#include "PTCommunicationService.h"

#include "Services/MqttBrokerService.h"

#include <thread>
#include <windows.h>

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
            std::thread clientThread = std::thread([&]{
                //sleep(2);
                ClientService->connectClientStartCommunication();
            });

            serverThread.join();
        }
        catch(std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    CommunicationService::CommunicationService(std::string , std::string ) {

    }

    void CommunicationService::addObservationCallbackForTopic(std::string topic, std::function<void(std::string)> callback) {
        ClientService->setCallbackFunction(topic,callback);
    }

    void CommunicationService::publishMQTTMessage(std::string topic, std::string content) {
        ClientService->sendValueToServer(topic, content);
    }
}