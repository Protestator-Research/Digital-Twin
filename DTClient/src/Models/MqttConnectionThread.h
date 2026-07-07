//
// Created by Moritz Herzog on 13.01.25.
//

#ifndef DIGITALTWIN_MQTTCONNECTIONTHREAD_H
#define DIGITALTWIN_MQTTCONNECTIONTHREAD_H

#include <string>
#include <functional>
//#include <QMqttClient>
#include <QObject>

#include <Services/IMqttClientService.h>

namespace PHYSICAL_TWIN_COMMUNICATION {
    class MqttClientService;
}

namespace DigitalTwin::Client {
    class MQTTConnectionThread : public QObject, public PHYSICAL_TWIN_COMMUNICATION::IMqttClientService{
        Q_OBJECT
    public:
        MQTTConnectionThread() = delete;
        MQTTConnectionThread(std::string url, std::string port, std::string username = "", std::string password = "");
        virtual ~MQTTConnectionThread() = default;

        void start();

        void addObserverForTopic(const std::string& topic, std::function<void(std::string)> callbackFunction);
        void publish(std::string topic, std::string value) override;

    private slots:
        void onConnect();
        void onDisconnect();
        void onErrorChanged();

    private:
       std::shared_ptr<PHYSICAL_TWIN_COMMUNICATION::MqttClientService> ClientService;
    };
}


#endif //DIGITALTWIN_MQTTCONNECTIONTHREAD_H
