//
// Created by Moritz Herzog on 25.04.24.
//

#include "DigitalTwinClientSettings.h"

#include <QCoreApplication>
#include <QStandardPaths>

namespace DigitalTwin::Client {
    DigitalTwinClientSettings::DigitalTwinClientSettings(QObject *parent) :
        Settings(new QSettings( QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/config.ini", QSettings::IniFormat)) {
        initAfterSoftwareStart();
    }

    void DigitalTwinClientSettings::initAfterSoftwareStart() {
        Settings->beginGroup(ServerGroupIdentifier);

        if (getRESTServer().isEmpty())
            resetToDefault();
    }

    void DigitalTwinClientSettings::resetToDefault() {
        Settings->setValue(RESTServerIdentifier, DefaultRESTServer);
        Settings->setValue(RESTPortIdentifier, DefaultRESTPort);
        Settings->setValue(MQTTServerIdentifier, DefaultMQTTServer);
        Settings->setValue(MQTTPortIdentifier, DefaultMQTTPort);
        Settings->setValue(RESTLoginIdentifier,DefaultRESTLogin);
        Settings->setValue(RESTPasswordIdentifier, DefaultRESTPassword);
        Settings->sync();
    }

    DigitalTwinClientSettings::~DigitalTwinClientSettings() {
        Settings->endGroup();
        Settings->sync();
        delete Settings;
    }

    QString DigitalTwinClientSettings::getRESTServer() {
        return Settings->value(RESTServerIdentifier).toString();
    }

    std::string DigitalTwinClientSettings::getRESTServerAsString() {
        return Settings->value(RESTServerIdentifier).toString().toStdString();
    }

    void DigitalTwinClientSettings::setRESTServer(QString server) {
        Settings->setValue(RESTServerIdentifier, server);
    }

    QString DigitalTwinClientSettings::getRESTPort() {
        return Settings->value(RESTPortIdentifier).toString();
    }

    std::string DigitalTwinClientSettings::getRESTPortAsString() {
        return Settings->value(RESTPortIdentifier).toString().toStdString();
    }

    void DigitalTwinClientSettings::setRESTPort(QString port) {
        Settings->setValue(RESTPortIdentifier, port);
    }

    QString DigitalTwinClientSettings::getMQTTServer() {
        return Settings->value(MQTTServerIdentifier).toString();
    }

    std::string DigitalTwinClientSettings::getMQTTServerAsString() {
        return Settings->value(MQTTServerIdentifier).toString().toStdString();
    }

    void DigitalTwinClientSettings::setMQTTServer(QString server) {
        Settings->setValue(MQTTServerIdentifier, server);
    }

    QString DigitalTwinClientSettings::getMQTTPort() {
        return Settings->value(MQTTPortIdentifier).toString();
    }

    std::string DigitalTwinClientSettings::getMQTTPortAsString() {
        return Settings->value(MQTTPortIdentifier).toString().toStdString();
    }

    void DigitalTwinClientSettings::setMQTTPort(QString port) {
        Settings->setValue(MQTTPortIdentifier, port);
    }

    void DigitalTwinClientSettings::setRESTUser(QString user) {
        Settings->setValue(RESTLoginIdentifier, user);
    }

    QString DigitalTwinClientSettings::getRESTUser() {
        return Settings->value(RESTLoginIdentifier).toString();
    }

    std::string DigitalTwinClientSettings::getRESTUserAsString() {
        return Settings->value(RESTLoginIdentifier).toString().toStdString();
    }

    void DigitalTwinClientSettings::setRESTPassword(QString password) {
        Settings->setValue(RESTPasswordIdentifier,password);
    }

    QString DigitalTwinClientSettings::getRESTPassword() {
        return Settings->value(RESTPasswordIdentifier).toString();
    }

    std::string DigitalTwinClientSettings::getRESTPasswordAsString() {
        return Settings->value(RESTPasswordIdentifier).toString().toStdString();
    }
} // DigitalTwin::Client