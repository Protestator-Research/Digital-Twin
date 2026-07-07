//
// Created by Moritz Herzog on 13.01.25.
//

#include "MqttConnectionThread.h"
#include <Services/MqttClientService.h>
#include <QDebug>

namespace DigitalTwin::Client {

    MQTTConnectionThread::MQTTConnectionThread(std::string url, std::string port, std::string username, std::string password)
    {
        ClientService = std::make_shared<PHYSICAL_TWIN_COMMUNICATION::MqttClientService>(new boost::asio::io_context(),url,port,"digital-twin-client");
        
        // TODO: Needs to be refitted to the new Client and username and password.
        //if (!username.empty())
            //ClientService->setUsername(QString::fromStdString(username));
        //if (!password.empty())
            //ClientService->setPassword(QString::fromStdString(password));

        //connect(ClientService, SIGNAL(connected()), this, SLOT(onConnect()));
        //connect(ClientService, SIGNAL(disconnected()),this, SLOT(onDisconnect()));
        //connect(ClientService, SIGNAL(errorChanged(QMqttClient::ClientError)),this, SLOT(onErrorChanged(QMqttClient::ClientError)));

        //ClientService(new PHYSICAL_TWIN_COMMUNICATION::MqttClientService(new boost::asio::io_context(),url, port, "digital-twin-client"))
    }


    void MQTTConnectionThread::start() {
        ClientService->start();
        //ClientService->connectToHost();
    }

    void MQTTConnectionThread::addObserverForTopic(const std::string &,
                                                   std::function<void(std::string)> ) {
        // ClientService->subscribe(topic,[callbackFunction](const std::string&, const std::string& payload)->void {callbackFunction(payload);});
    }

    void MQTTConnectionThread::publish(std::string topic, std::string value) {
        //[[maybe_unused]] auto messageId = ClientService->publish(QMqttTopicName(QString::fromStdString(topic)),QByteArray(value));
        //qDebug()<<messageId;
    }

    void MQTTConnectionThread::onConnect() {
        qDebug()<<"MQTTConnectionThread::onConnect()";
    }

    void MQTTConnectionThread::onDisconnect() {
        qDebug()<<"MQTTConnectionThread::onDisconnect()";
    }

    void MQTTConnectionThread::onErrorChanged() {
        qDebug()<<"MQTTConnectionThread::onErrorChanged()";
    }
}
