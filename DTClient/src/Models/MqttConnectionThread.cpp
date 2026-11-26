//
// Created by Moritz Herzog on 13.01.25.
//

#include "MqttConnectionThread.h"
#include <Services/MqttClientService.h>
#include <QDebug>
#include <QMetaObject>

namespace DigitalTwin::Client {

    MQTTConnectionThread::MQTTConnectionThread(std::string url, std::string port, DigitalTwin::Communication::MqttProtocolVersion version,
                                               std::string username, std::string password,
                                               DigitalTwin::Communication::MqttTlsOptions tls) :
        // An empty client id: the service makes up a unique one. A fixed id would let two clients on one broker take
        // each other's session over (the broker allows one connection per client id).
        Service(std::make_unique<DigitalTwin::Communication::MqttClientService>(std::move(url), std::move(port), "", version,
                                                                                std::move(username), std::move(password), std::move(tls)))
    {
        connect(this, SIGNAL(connected()), this, SLOT(onConnect()));
        // Called on a thread of the Paho library; the signal is delivered in the thread of this object.
        // The raw pointer, not Service: while Service is reset in the destructor the unique_ptr is already null,
        // but the handler may still run until the service has finished its callbacks.
        DigitalTwin::Communication::MqttClientService* service = Service.get();
        Service->setErrorHandler([this, service](const std::string& message) {
            qWarning() << QString::fromStdString(message);
            if (service->isAuthenticationFailed())
                QMetaObject::invokeMethod(this, [this]() { emit loginRefused(); }, Qt::QueuedConnection);
            else if (service->hasTlsProblem() && !TlsProblemReported.exchange(true)) {
                const QString text = QString::fromStdString(message);
                QMetaObject::invokeMethod(this, [this, text]() { emit tlsProblem(text); }, Qt::QueuedConnection);
            }
        });
        Service->setConnectedHandler([this]() {
            TlsProblemReported = false;
            QMetaObject::invokeMethod(this, [this]() { emit connected(); }, Qt::QueuedConnection);
        });
    }

    MQTTConnectionThread::~MQTTConnectionThread() {
        // Waits for the Paho callbacks, which use this object, before it is destroyed.
        Service.reset();
    }

    void MQTTConnectionThread::start() {
        Service->start();
    }

    bool MQTTConnectionThread::isLoginRefused() const {
        return Service->isAuthenticationFailed();
    }

    bool MQTTConnectionThread::hasTlsProblem() const {
        return Service->hasTlsProblem();
    }

    void MQTTConnectionThread::addObserverForTopic(const std::string& topic,
                                                   std::function<void(std::string)> callbackFunction) {
        subscribe(topic, [callbackFunction](const std::string&, const std::string& payload) -> void {
            callbackFunction(payload);
        });
    }

    void MQTTConnectionThread::publish(std::string topic, std::string value) {
        Service->publish(std::move(topic), std::move(value));
    }

    void MQTTConnectionThread::subscribe(std::string topic, std::function<void(std::string topic, std::string payload)> callback) {
        Service->subscribe(std::move(topic), std::move(callback));
    }

    void MQTTConnectionThread::onConnect() {
        qDebug()<<"MQTTConnectionThread::onConnect()";
        Connected = true;
    }
}
