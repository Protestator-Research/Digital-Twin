//
// Created by Moritz Herzog on 23.05.24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_DigitalTwinTabWidget.h" resolved
#include "DigitalTwinTabWidget.h"
#include "../../ui/ui_DigitalTwinTabWidget.h"

#include <Model/DigitalTwinModel.h>
#include <QString>

namespace DigitalTwin::Client {

    DigitalTwinTabWidget::DigitalTwinTabWidget(Model::DigitalTwinModel* model, QWidget *parent) :
            QWidget(parent), ui(new Ui::DigitalTwinTabWidget) {
        ui->setupUi(this);
        Model = model;
        ui->DigitalTwinNamLabel->setText(QString::fromStdString(Model->digitalTwinName()));
        makeConnections();
    }

    DigitalTwinTabWidget::~DigitalTwinTabWidget() {
        delete ui;
    }

    void DigitalTwinTabWidget::makeConnections() {

    }
}