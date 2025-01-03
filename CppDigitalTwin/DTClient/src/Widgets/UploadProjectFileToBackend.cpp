//
// Created by Moritz Herzog on 03.01.25.
//

#include "UploadProjectFileToBackend.h"
#include "ui_UploadProjectFileToBackend.h"


namespace DigitalTwin::Client {
    UploadProjectFileToBackend::UploadProjectFileToBackend(QWidget *parent) :
            QMainWindow(parent),
            Ui(new Ui::UploadProjectFileToBackend())
    {
        Ui->setupUi(this);
        Ui->retranslateUi(this);
    }

    UploadProjectFileToBackend::~UploadProjectFileToBackend() {
        delete Ui;
    }

}