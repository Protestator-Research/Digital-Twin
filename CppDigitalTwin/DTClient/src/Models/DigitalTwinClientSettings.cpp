//
// Created by Moritz Herzog on 25.04.24.
//

#include "DigitalTwinClientSettings.h"

#include <QCoreApplication>

namespace DigitalTwin::Client {
    DigitalTwinClientSettings::DigitalTwinClientSettings(QObject *parent) :
            Settings(new QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent)) {
        initAfterSoftwareStart();
    }

    void DigitalTwinClientSettings::initAfterSoftwareStart() {
        Settings->beginGroup(ServerGroupIdentifier);

        if (!Settings->contains(RESTServerIdentifier))
            resetToDefault();
    }

    void DigitalTwinClientSettings::resetToDefault() {
        Settings->setProperty(RESTServerIdentifier, DefaultRESTServer);
        Settings->setProperty(RESTPortIdentifier, DefaultRESTPort);
        Settings->setProperty(MQTTServerIdentifier, DefaultMQTTServer);
        Settings->setProperty(MQTTPortIdentifier, DefaultMQTTPort);
        Settings->setProperty(ServerEqualityIdentifier, DefaultRESTAndMQTTServerAreEqual);
    }

    DigitalTwinClientSettings::~DigitalTwinClientSettings() {
        Settings->endGroup();
        delete Settings;
    }

    QString DigitalTwinClientSettings::getRESTServer() {
        return Settings->property(RESTServerIdentifier).toString();
    }

    std::string DigitalTwinClientSettings::getRESTServerAsString() {
        return Settings->property(RESTServerIdentifier).toString().toStdString();
    }

    void DigitalTwinClientSettings::setRESTServer(QString server) {
        Settings->setProperty(RESTServerIdentifier, server);
    }

    QString DigitalTwinClientSettings::getRESTPort() {
        return Settings->property(RESTPortIdentifier).toString();
    }

    std::string DigitalTwinClientSettings::getRESTPortAsString() {
        return Settings->property(RESTPortIdentifier).toString().toStdString();
    }

    void DigitalTwinClientSettings::setRESTPort(QString port) {
        Settings->setProperty(RESTPortIdentifier, port);
    }

    QString DigitalTwinClientSettings::getMQTTServer() {
        return Settings->property(MQTTServerIdentifier).toString();
    }

    std::string DigitalTwinClientSettings::getMQTTServerAsString() {
        return Settings->property(MQTTServerIdentifier).toString().toStdString();
    }

    void DigitalTwinClientSettings::setMQTTServer(QString server) {
        Settings->setProperty(MQTTServerIdentifier, server);
    }

    QString DigitalTwinClientSettings::getMQTTPort() {
        return Settings->property(MQTTPortIdentifier).toString();
    }

    std::string DigitalTwinClientSettings::getMQTTPortAsString() {
        return Settings->property(MQTTPortIdentifier).toString().toStdString();
    }

    void DigitalTwinClientSettings::setMQTTPort(QString port) {
        Settings->setProperty(MQTTPortIdentifier, port);
    }

    bool DigitalTwinClientSettings::getRESTandMQTTServerAreEqual() {
        return Settings->property(ServerEqualityIdentifier).toBool();
    }

    void DigitalTwinClientSettings::setRESTandMQTTSererAreEqual(bool equality) {
        Settings->setProperty(ServerEqualityIdentifier, equality);
    }
} // DigitalTwin::Client