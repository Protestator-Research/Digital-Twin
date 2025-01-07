//
// Created by Moritz Herzog on 07.01.25.
//

// You may need to build the project (run Qt uic code generator) to get "ui_CreateProjectDialog.h" resolved

#include "CreateProjectDialog.h"
#include "ui_CreateProjectDialog.h"

namespace DigitalTwin::Client {
    CreateProjectDialog::CreateProjectDialog(QWidget *parent) :
            QDialog(parent),
            Ui(new Ui::CreateProjectDialog) {
        Ui->setupUi(this);

        connect(Ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(Ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    }

    CreateProjectDialog::~CreateProjectDialog() {
        delete Ui;
    }

    std::string CreateProjectDialog::getProjectName() {
        return Ui->ProjectNameLineEdit->text().toStdString();
    }

    std::string CreateProjectDialog::getProjectDecription() {
        return Ui->ProjectDescriptionTextEdit->toPlainText().toStdString();
    }
} // DigitalTwin::Client
