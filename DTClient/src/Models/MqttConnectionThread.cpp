//
// Created by Moritz Herzog on 13.01.25.
//

#include "MqttConnectionThread.h"
#include <Services/MqttClientService.h>

namespace DigitalTwin::Client {

    MQTTConnectionThread::MQTTConnectionThread(std::string url, std::string port, QObject* parent) :
            QThread(parent),
            ClientService(new PHYSICAL_TWIN_COMMUNICATION::MqttClientService(new boost::asio::io_context(),url, port, "digital-twin-client")){
    }

    MQTTConnectionThread::~MQTTConnectionThread() {
        delete ClientService;
    }

    void MQTTConnectionThread::run() {
        ClientService->start();
        emit resultReady();
    }

    void MQTTConnectionThread::addObserverForTopic(const std::string &topic,
                                                   std::function<void(std::string)> callbackFunction) {
        ClientService->subscribe(topic,[callbackFunction](const std::string&, const std::string& payload)->void {callbackFunction(payload);});
    }

    PHYSICAL_TWIN_COMMUNICATION::MqttClientService* MQTTConnectionThread::getClientService() {
        return ClientService;
    }

    void MQTTConnectionThread::sendValueToSever(const std::string &topic, std::string value) {
        ClientService->publish(topic,value);
    }
}