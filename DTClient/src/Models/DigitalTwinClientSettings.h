//
// Created by Moritz Herzog on 25.04.24.
//

#ifndef DIGITALTWIN_DIGITALTWINCLIENTSETTINGS_H
#define DIGITALTWIN_DIGITALTWINCLIENTSETTINGS_H

#include <QSettings>

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
         * @param parent
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

        void setRESTUser(QString user);

        QString getRESTUser();

        std::string getRESTUserAsString();

        void setRESTPassword(QString password);

        QString getRESTPassword();

        std::string getRESTPasswordAsString();

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

        void initAfterSoftwareStart();
    };
} // DigitalTwin::Client

#endif //DIGITALTWIN_DIGITALTWINCLIENTSETTINGS_H
