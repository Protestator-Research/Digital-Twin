//
// Created by Moritz Herzog on 25.04.24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_SettingsDialog.h" resolved

#include "SettingsDialog.h"
#include "../../ui/ui_SettingsDialog.h"

namespace DigitalTwin::Client {
    SettingsDialog::SettingsDialog(QWidget *parent) :
            QDialog(parent), ui(new Ui::SettingsDialog) {
        ui->setupUi(this);
    }

    SettingsDialog::~SettingsDialog() {
        delete ui;
    }
} // DigitalTwin::Client
