//
// Created by Moritz Herzog on 13.01.25.
//

#ifndef DIGITALTWIN_MQTTCONNECTIONTHREAD_H
#define DIGITALTWIN_MQTTCONNECTIONTHREAD_H

#include <QThread>

namespace PHYSICAL_TWIN_COMMUNICATION {
    class MqttClientService;
}

namespace DigitalTwin::Client {
    class MQTTConnectionThread : public QThread {
    Q_OBJECT
    public:
        MQTTConnectionThread() = delete;
        MQTTConnectionThread(std::string url, std::string port, QObject* parent = nullptr);
        ~MQTTConnectionThread();

        void run() override;

        void addObserverForTopic(const std::string& topic, std::function<void(std::string)> callbackFunction);
        PHYSICAL_TWIN_COMMUNICATION::MqttClientService* getClientService();
        void sendValueToSever(const std::string& topic, std::string value);
    signals:
        void resultReady();

    private:
        PHYSICAL_TWIN_COMMUNICATION::MqttClientService* ClientService;
    };
}


#endif //DIGITALTWIN_MQTTCONNECTIONTHREAD_H
