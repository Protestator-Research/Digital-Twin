//
// Created by Moritz Herzog on 25.04.24.
//

#ifndef DIGITALTWIN_SETTINGSDIALOG_H
#define DIGITALTWIN_SETTINGSDIALOG_H

#include <QDialog>
#include "../../Models/DigitalTwinClientSettings.h"

namespace DigitalTwin::Client {
    QT_BEGIN_NAMESPACE
    namespace Ui {
        class SettingsDialog;
    }
    QT_END_NAMESPACE

    class SettingsDialog : public QDialog {
    Q_OBJECT

    public:
        explicit SettingsDialog(DigitalTwinClientSettings *settings, QWidget *parent = nullptr);

        ~SettingsDialog() override;

    private:
        Ui::SettingsDialog *Ui;
        DigitalTwinClientSettings *Settings;
        // The own MQTT login as typed by the user, kept while the fields show the REST login.
        QString OwnMqttUser;
        QString OwnMqttPassword;
        void makeConnections();
        void initSettings();

    private slots:
        void lockServerURL();
        void lockMqttLogin();
        void chooseCaFile();
        void onTextChangeOfSysMLURLLineEdit(const QString &);
        void onButtonAcceptedClicked();
        void resetToDefaults();
    };
} // DigitalTwin::Client

#endif //DIGITALTWIN_SETTINGSDIALOG_H
