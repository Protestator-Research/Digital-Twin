//
// Created by Moritz Herzog on 25.04.24.
//

#include "DigitalTwinClientSettings.h"

#include <QCoreApplication>
#include <QStandardPaths>

namespace DigitalTwin::Client {
    DigitalTwinClientSettings::DigitalTwinClientSettings(QObject *) :

        Settings(new QSettings( QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/config.ini", QSettings::IniFormat)) {
        initAfterSoftwareStart();
    }

    void DigitalTwinClientSettings::initAfterSoftwareStart() {
        Settings->beginGroup(ServerGroupIdentifier);

        if (getRESTServer().isEmpty())
            resetToDefault();
        if (getRESTFolder().isEmpty())
            Settings->setValue(RESTFolderIdentifier,DefaultRestFolder);
        // Settings of an older version do not have the MQTT protocol version yet.
        if (!Settings->contains(MQTTProtocolVersionIdentifier))
            Settings->setValue(MQTTProtocolVersionIdentifier, DefaultMQTTProtocolVersion);
        // Settings of an older version used the REST login for MQTT.
        if (!Settings->contains(MQTTUseRESTLoginIdentifier))
            Settings->setValue(MQTTUseRESTLoginIdentifier, DefaultMQTTUseRESTLogin);
    }

    void DigitalTwinClientSettings::resetToDefault() {
        Settings->setValue(RESTServerIdentifier, DefaultRESTServer);
        Settings->setValue(RESTPortIdentifier, DefaultRESTPort);
        Settings->setValue(MQTTServerIdentifier, DefaultMQTTServer);
        Settings->setValue(MQTTPortIdentifier, DefaultMQTTPort);
        Settings->setValue(MQTTProtocolVersionIdentifier, DefaultMQTTProtocolVersion);
        Settings->setValue(MQTTUseRESTLoginIdentifier, DefaultMQTTUseRESTLogin);
        Settings->setValue(MQTTLoginIdentifier, DefaultMQTTLogin);
        Settings->setValue(MQTTPasswordIdentifier, DefaultMQTTPassword);
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

    QString DigitalTwinClientSettings::getMQTTProtocolVersionAsText() {
        return Settings->value(MQTTProtocolVersionIdentifier, DefaultMQTTProtocolVersion).toString() == "5" ? "5" : "3.1.1";
    }

    DigitalTwin::Communication::MqttProtocolVersion DigitalTwinClientSettings::getMQTTProtocolVersion() {
        return getMQTTProtocolVersionAsText() == "5" ? DigitalTwin::Communication::MqttProtocolVersion::V5
                                                     : DigitalTwin::Communication::MqttProtocolVersion::V3_1_1;
    }

    void DigitalTwinClientSettings::setMQTTProtocolVersion(QString version) {
        Settings->setValue(MQTTProtocolVersionIdentifier, version == "5" ? "5" : "3.1.1");
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

    bool DigitalTwinClientSettings::getMQTTUseRESTLogin() {
        return Settings->value(MQTTUseRESTLoginIdentifier, DefaultMQTTUseRESTLogin).toBool();
    }

    void DigitalTwinClientSettings::setMQTTUseRESTLogin(bool useRESTLogin) {
        Settings->setValue(MQTTUseRESTLoginIdentifier, useRESTLogin);
    }

    QString DigitalTwinClientSettings::getMQTTUser() {
        return Settings->value(MQTTLoginIdentifier, DefaultMQTTLogin).toString();
    }

    void DigitalTwinClientSettings::setMQTTUser(QString user) {
        Settings->setValue(MQTTLoginIdentifier, user);
    }

    QString DigitalTwinClientSettings::getMQTTPassword() {
        return Settings->value(MQTTPasswordIdentifier, DefaultMQTTPassword).toString();
    }

    void DigitalTwinClientSettings::setMQTTPassword(QString password) {
        Settings->setValue(MQTTPasswordIdentifier, password);
    }

    std::string DigitalTwinClientSettings::getMQTTLoginUserAsString() {
        return getMQTTUseRESTLogin() ? getRESTUserAsString() : getMQTTUser().toStdString();
    }

    std::string DigitalTwinClientSettings::getMQTTLoginPasswordAsString() {
        return getMQTTUseRESTLogin() ? getRESTPasswordAsString() : getMQTTPassword().toStdString();
    }

    QString DigitalTwinClientSettings::getRESTFolder() {
        return Settings->value(RESTFolderIdentifier).toString();
    }

    void DigitalTwinClientSettings::setRESTFolder(QString folder) {
        Settings->setValue(RESTFolderIdentifier, folder);
    }

    std::string DigitalTwinClientSettings::getRESTFolderAsString() {
        return Settings->value(RESTFolderIdentifier).toString().toStdString();
    }
} // DigitalTwin::Client