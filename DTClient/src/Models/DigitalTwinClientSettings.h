//
// Created by Moritz Herzog on 25.04.24.
//

#ifndef DIGITALTWIN_DIGITALTWINCLIENTSETTINGS_H
#define DIGITALTWIN_DIGITALTWINCLIENTSETTINGS_H

#include <QSettings>
#include <Services/IMqttClientService.h>

namespace DigitalTwin::Client {
    /**
     * Represents the Model to read an write the Digital Twin Client settings.
     * @author Moritz Herzog <herzogm@rptu.de>
     * @class DigitalTwinClientSettings
     * @version 1.0
     */
    class DigitalTwinClientSettings {
    public:
        /**
         * Constructor
         * @param parent The parent object, so that the dialog is destroyed, if the parent is destroyed.
         */
        DigitalTwinClientSettings(QObject *parent = nullptr);

        ~DigitalTwinClientSettings();

        void resetToDefault();

        QString getRESTServer();

        std::string getRESTServerAsString();

        void setRESTServer(QString server);

        QString getRESTPort();

        std::string getRESTPortAsString();

        void setRESTPort(QString port);

        QString getMQTTServer();

        std::string getMQTTServerAsString();

        void setMQTTServer(QString server);

        QString getMQTTPort();

        std::string getMQTTPortAsString();

        void setMQTTPort(QString port);

        /**
         * The MQTT protocol version as text: "3.1.1" or "5".
         */
        QString getMQTTProtocolVersionAsText();

        DigitalTwin::Communication::MqttProtocolVersion getMQTTProtocolVersion();

        /**
         * @param version "3.1.1" or "5"; anything else is stored as "3.1.1".
         */
        void setMQTTProtocolVersion(QString version);

        void setRESTUser(QString user);

        QString getRESTUser();

        std::string getRESTUserAsString();

        void setRESTPassword(QString password);

        QString getRESTPassword();

        std::string getRESTPasswordAsString();

        /**
         * Whether the MQTT login uses the same user and password as the SysML v2 REST server.
         */
        bool getMQTTUseRESTLogin();

        void setMQTTUseRESTLogin(bool useRESTLogin);

        /**
         * The stored own MQTT user (only used if the REST login is not used for MQTT).
         */
        QString getMQTTUser();

        void setMQTTUser(QString user);

        /**
         * The stored own MQTT password (only used if the REST login is not used for MQTT).
         */
        QString getMQTTPassword();

        void setMQTTPassword(QString password);

        /**
         * The effective MQTT user: the REST user, if the REST login is used, otherwise the own MQTT user.
         */
        std::string getMQTTLoginUserAsString();

        /**
         * The effective MQTT password: the REST password, if the REST login is used, otherwise the own MQTT password.
         */
        std::string getMQTTLoginPasswordAsString();

        /**
         * Whether the MQTT connection uses TLS (ssl://). Off by default. The certificate of the server is always
         * verified, including its host name. A TLS scheme (ssl://, mqtts://, tls://) typed into the MQTT server
         * field turns TLS on as well, whatever this says, and a plain scheme (tcp://, mqtt://) does not turn it
         * off: TLS wins (see makeMqttEndpoint()).
         */
        bool getMQTTUseTLS();

        void setMQTTUseTLS(bool useTls);

        /**
         * A PEM file with the CA certificates that are trusted for the MQTT server, or empty. If it is set, it
         * REPLACES the certificates of the operating system (for a private CA or for tests). Only used with TLS.
         */
        QString getMQTTCaFile();

        void setMQTTCaFile(QString file);

        void setRESTFolder(QString folder);
        QString getRESTFolder();
        std::string getRESTFolderAsString();

    private:
        QSettings *Settings;

        const QString ServerGroupIdentifier = "Connection Settings";

        const char *RESTServerIdentifier = "RESTServer";
        const QString DefaultRESTServer = "localhost";
        const char *RESTPortIdentifier = "RESTPort";
        const QString DefaultRESTPort = "8080";
        const char *RESTLoginIdentifier = "RESTLogin";
        const QString DefaultRESTLogin = "admin@cps.de";
        const char *RESTFolderIdentifier = "RESTFolder";
        const QString DefaultRestFolder = "/agila-server/";
        const char *RESTPasswordIdentifier = "RESTPassword";
        const QString DefaultRESTPassword = "admin";
        const char *MQTTServerIdentifier = "MQTTServer";
        const QString DefaultMQTTServer = "localhost";
        const char *MQTTPortIdentifier = "MQTTPort";
        const QString DefaultMQTTPort = "1883";
        const char *MQTTProtocolVersionIdentifier = "MQTTProtocolVersion";
        const QString DefaultMQTTProtocolVersion = "3.1.1";
        const char *MQTTUseRESTLoginIdentifier = "MQTTUseRESTLogin";
        const bool DefaultMQTTUseRESTLogin = true;
        const char *MQTTLoginIdentifier = "MQTTLogin";
        const QString DefaultMQTTLogin = "";
        const char *MQTTPasswordIdentifier = "MQTTPassword";
        const QString DefaultMQTTPassword = "";
        const char *MQTTUseTLSIdentifier = "MQTTUseTLS";
        const bool DefaultMQTTUseTLS = false;
        const char *MQTTCaFileIdentifier = "MQTTCaFile";
        const QString DefaultMQTTCaFile = "";

        void initAfterSoftwareStart();
    };
} // DigitalTwin::Client

#endif //DIGITALTWIN_DIGITALTWINCLIENTSETTINGS_H
