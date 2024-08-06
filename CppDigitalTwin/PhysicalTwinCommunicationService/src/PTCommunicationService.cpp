//
// Created by Moritz Herzog on 17.01.24.
//

#include "PTCommunicationService.h"

#include "Services/MqttBrokerService.h"



namespace PHYSICAL_TWIN_COMMUNICATION {

    CommunicationService::CommunicationService(std::string mqttPort) {
        MqttPort=std::stoi(mqttPort);
        startThreads();
    }

    void CommunicationService::startThreads() {
        try {
            MQTTBrokerService::runBroker(MqttPort);
        }
        catch(std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

}