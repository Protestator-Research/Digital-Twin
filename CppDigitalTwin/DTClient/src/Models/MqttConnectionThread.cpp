//
// Created by Moritz Herzog on 13.01.25.
//

#include "MqttConnectionThread.h"
#include <Services/MqttClientService.h>

namespace DigitalTwin::Client {

    MQTTConnectionThread::MQTTConnectionThread(std::string url, std::string port, QObject* parent) :
            QThread(parent),
            ClientService(new PHYSICAL_TWIN_COMMUNICATION::MqttClientService(url,port)){
    }

    MQTTConnectionThread::~MQTTConnectionThread() {
        delete ClientService;
    }

    void MQTTConnectionThread::run() {
        QThread::run();
        ClientService->connectClientStartCommunication();
        emit resultReady();
    }

    void MQTTConnectionThread::addObserverForTopic(const std::string &topic,
                                                   std::function<void(std::string)> callbackFunction) {
        ClientService->addCallbackFunction(topic,callbackFunction);

    }

    PHYSICAL_TWIN_COMMUNICATION::MqttClientService* MQTTConnectionThread::getClientService() {
        return ClientService;
    }
}