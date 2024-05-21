//
// Created by Moritz Herzog on 25.04.24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_SettingsDialog.h" resolved

#include <QPushButton>

#include "SettingsDialog.h"
#include "../../../ui/ui_SettingsDialog.h"

namespace DigitalTwin::Client {
    SettingsDialog::SettingsDialog(DigitalTwinClientSettings *settings, QWidget *parent) :
            QDialog(parent), Ui(new Ui::SettingsDialog) {
        Ui->setupUi(this);
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

    void SettingsDialog::makeConnections() {
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
        Settings->setRESTServer(Ui->SysMLServerURL->text());
        Settings->setRESTPort(Ui->SysMLServerPort->text());
        Settings->setRESTPassword(Ui->PasswordLineEdit->text());
        Settings->setRESTUser(Ui->UserLineEdit->text());
        accept();
    }

    void SettingsDialog::initSettings() {
        Ui->SysMLServerURL->setText(Settings->getRESTServer());
        Ui->SysMLServerPort->setText(Settings->getRESTPort());
        Ui->SameNameCheckBox->setChecked(Settings->getRESTServer()==Settings->getMQTTServer());
        Ui->DTServerLineEdit->setText(Settings->getMQTTServer());
        Ui->DTPortLineEdit->setText(Settings->getMQTTPort());
        Ui->UserLineEdit->setText(Settings->getRESTUser());
        Ui->PasswordLineEdit->setText(Settings->getRESTPassword());
    }

    void SettingsDialog::resetToDefaults() {
        Settings->resetToDefault();
        initSettings();
    }
} // DigitalTwin::Client
