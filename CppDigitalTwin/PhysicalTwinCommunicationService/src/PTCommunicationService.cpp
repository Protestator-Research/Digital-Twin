//
// Created by Moritz Herzog on 17.01.24.
//

#include "PTCommunicationService.h"

#include "Services/MqttBrokerService.h"

#include <thread>


namespace PHYSICAL_TWIN_COMMUNICATION {

    CommunicationService::CommunicationService(std::string mqttPort) {
        MqttPort=std::stoi(mqttPort);
        ClientService = std::make_unique<MqttClientService>("localhost",mqttPort);
        startThreads();
    }

    void CommunicationService::startThreads() {
        try {
            bool serverStarted = false;
            std::thread serverThread = std::thread([&]{
                MQTTBrokerService::runBroker(MqttPort,serverStarted);
            });
            std::thread clientThread = std::thread([&]{
                sleep(2);
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

}