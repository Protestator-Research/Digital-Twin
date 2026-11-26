//
// Created by Moritz Herzog on 25.04.24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_SettingsDialog.h" resolved

#include <QFileDialog>
#include <QPushButton>

#include "SettingsDialog.h"

#include "ui_SettingsDialog.h"


namespace DigitalTwin::Client {
    SettingsDialog::SettingsDialog(DigitalTwinClientSettings *settings, QWidget *parent) :
            QDialog(parent), Ui(new Ui::SettingsDialog) {
        Ui->setupUi(this);
        // The values stored in the settings; the texts of the entries are for the user.
        Ui->MqttVersionComboBox->setItemData(0, "3.1.1");
        Ui->MqttVersionComboBox->setItemData(1, "5");
        makeConnections();
        Settings = settings;
        initSettings();
    }

    SettingsDialog::~SettingsDialog() {
        delete Ui;
    }

    void SettingsDialog::lockServerURL() {
        if(Ui->SameNameCheckBox->isChecked()) {
            Ui->DTServerLineEdit->setText(Ui->SysMLServerURL->text());
            Ui->DTServerLineEdit->setEnabled(false);
        }
        else {
            Ui->DTServerLineEdit->setText("");
            Ui->DTServerLineEdit->setEnabled(true);
        }
    }

    void SettingsDialog::lockMqttLogin() {
        // Must not use Settings: it is connected before Settings is set.
        if(Ui->SameLoginCheckBox->isChecked()) {
            Ui->DTUserLineEdit->setText(Ui->UserLineEdit->text());
            Ui->DTPasswordLineEdit->setText(Ui->PasswordLineEdit->text());
            Ui->DTUserLineEdit->setEnabled(false);
            Ui->DTPasswordLineEdit->setEnabled(false);
        }
        else {
            Ui->DTUserLineEdit->setText(OwnMqttUser);
            Ui->DTPasswordLineEdit->setText(OwnMqttPassword);
            Ui->DTUserLineEdit->setEnabled(true);
            Ui->DTPasswordLineEdit->setEnabled(true);
        }
    }

    void SettingsDialog::chooseCaFile() {
        const QString file = QFileDialog::getOpenFileName(this, tr("CA file"), Ui->CaFileLineEdit->text(),
                                                          tr("PEM certificates (*.pem *.crt *.cer);;All files (*)"));
        if(!file.isEmpty())
            Ui->CaFileLineEdit->setText(file);
    }

    void SettingsDialog::makeConnections() {
        connect(Ui->CaFileBrowseButton, SIGNAL(clicked(bool)), this, SLOT(chooseCaFile()));
        connect(Ui->SameLoginCheckBox, SIGNAL(toggled(bool)), this, SLOT(lockMqttLogin()));
        // The REST login is mirrored only while it is used for MQTT; the own login is not rewritten by it.
        auto mirrorRestLogin = [this]() {
            if(Ui->SameLoginCheckBox->isChecked())
                lockMqttLogin();
        };
        connect(Ui->UserLineEdit, &QLineEdit::textChanged, this, mirrorRestLogin);
        connect(Ui->PasswordLineEdit, &QLineEdit::textChanged, this, mirrorRestLogin);
        // textEdited is only sent for the input of the user, not for setText(); so the own login is not overwritten
        // by the mirrored REST login.
        connect(Ui->DTUserLineEdit, &QLineEdit::textEdited, this, [this](const QString &text) { OwnMqttUser = text; });
        connect(Ui->DTPasswordLineEdit, &QLineEdit::textEdited, this, [this](const QString &text) { OwnMqttPassword = text; });
        connect(Ui->SameNameCheckBox,SIGNAL(checkStateChanged(Qt::CheckState)),this, SLOT(lockServerURL()));
        connect(Ui->SysMLServerURL, SIGNAL(textChanged(QString)), this, SLOT(onTextChangeOfSysMLURLLineEdit(QString)));
        connect(Ui->buttonBox->button(QDialogButtonBox::StandardButton::Save), SIGNAL(clicked(bool)), this, SLOT(onButtonAcceptedClicked()));
        connect(Ui->buttonBox->button(QDialogButtonBox::StandardButton::Discard), SIGNAL(clicked(bool)),this,SLOT(reject()));
        connect(Ui->buttonBox->button(QDialogButtonBox::StandardButton::RestoreDefaults), SIGNAL(clicked(bool)),this,SLOT(resetToDefaults()));
    }

    void SettingsDialog::onTextChangeOfSysMLURLLineEdit(const QString &text) {
        if(Ui->SameNameCheckBox->isChecked())
            Ui->DTServerLineEdit->setText(text);

    }

    void SettingsDialog::onButtonAcceptedClicked() {
        Settings->setMQTTServer(Ui->DTServerLineEdit->text());
        Settings->setMQTTPort(Ui->DTPortLineEdit->text());
        Settings->setMQTTProtocolVersion(Ui->MqttVersionComboBox->currentData().toString());
        Settings->setMQTTUseTLS(Ui->UseTlsCheckBox->isChecked());
        Settings->setMQTTCaFile(Ui->CaFileLineEdit->text());
        Settings->setRESTServer(Ui->SysMLServerURL->text());
        Settings->setRESTPort(Ui->SysMLServerPort->text());
        Settings->setRESTPassword(Ui->PasswordLineEdit->text());
        Settings->setRESTUser(Ui->UserLineEdit->text());
        Settings->setRESTFolder(Ui->FolderLineEdit->text());
        Settings->setMQTTUseRESTLogin(Ui->SameLoginCheckBox->isChecked());
        // The REST login is not duplicated; the stored own login stays as it is, if it is not used.
        if(!Ui->SameLoginCheckBox->isChecked()) {
            Settings->setMQTTUser(Ui->DTUserLineEdit->text());
            Settings->setMQTTPassword(Ui->DTPasswordLineEdit->text());
        }
        accept();
    }

    void SettingsDialog::initSettings() {
        Ui->SysMLServerURL->setText(Settings->getRESTServer());
        Ui->SysMLServerPort->setText(Settings->getRESTPort());
        Ui->SameNameCheckBox->setChecked(Settings->getRESTServer()==Settings->getMQTTServer());
        Ui->DTServerLineEdit->setText(Settings->getMQTTServer());
        Ui->DTPortLineEdit->setText(Settings->getMQTTPort());
        Ui->MqttVersionComboBox->setCurrentIndex(qMax(0, Ui->MqttVersionComboBox->findData(Settings->getMQTTProtocolVersionAsText())));
        Ui->UseTlsCheckBox->setChecked(Settings->getMQTTUseTLS());
        Ui->CaFileLineEdit->setText(Settings->getMQTTCaFile());
        Ui->UserLineEdit->setText(Settings->getRESTUser());
        Ui->PasswordLineEdit->setText(Settings->getRESTPassword());
        Ui->FolderLineEdit->setText(Settings->getRESTFolder());
        OwnMqttUser = Settings->getMQTTUser();
        OwnMqttPassword = Settings->getMQTTPassword();
        Ui->SameLoginCheckBox->setChecked(Settings->getMQTTUseRESTLogin());
        // setChecked sends no signal, if the state does not change.
        lockMqttLogin();
    }

    void SettingsDialog::resetToDefaults() {
        Settings->resetToDefault();
        initSettings();
    }
} // DigitalTwin::Client
