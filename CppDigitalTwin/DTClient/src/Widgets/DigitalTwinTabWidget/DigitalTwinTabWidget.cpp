//
// Created by Moritz Herzog on 23.05.24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_DigitalTwinTabWidget.h" resolved
#include "DigitalTwinTabWidget.h"
#include "TabContentWidget.h"
#include "../TreeViewModels/VariableViewModel.h"
#include "../../ui_DigitalTwinTabWidget.h"
#include "../TreeViewModels/VariableViewModelItem.h"

#include <Model/DigitalTwinModel.h>
#include <QString>

namespace DigitalTwin::Client {

    DigitalTwinTabWidget::DigitalTwinTabWidget(Model::DigitalTwinModel* model, QWidget *parent) :
            QWidget(parent),
            ui(new Ui::DigitalTwinTabWidget),
            VariableModel(new VariableViewModel()) {
        ui->setupUi(this);
        Model = model;
        makeConnections();
        ui->VariableListView->setModel(VariableModel);
        generateModel();
    }

    DigitalTwinTabWidget::~DigitalTwinTabWidget() {
        delete ui;

        for(auto element : TabContents)
            delete element;

        TabContents.clear();
    }

    void DigitalTwinTabWidget::makeConnections() {
        connect(ui->VariableListView, SIGNAL(clicked(QModelIndex)), this, SLOT(onVariableClicked(QModelIndex)));
    }

    void DigitalTwinTabWidget::generateModel() {
        VariableModel->setDigitalTwinModel(Model);
        ui->VariableListView->expandAll();

        for(int i = 0; i<ui->VariableStackedWidget->count(); i++)
            ui->VariableStackedWidget->removeWidget(ui->VariableStackedWidget->widget(i));

    }

    void DigitalTwinTabWidget::onVariableClicked(const QModelIndex &index) {
//        auto element = VariableModel->getItemFromIndex(index);
        auto variableWidget = new TabContentWidget(this);
        TabContents.push_back(variableWidget);
        ui->VariableStackedWidget->addWidget(variableWidget);
        ui->VariableStackedWidget->setCurrentIndex(ui->VariableStackedWidget->count()-1);
    }
}