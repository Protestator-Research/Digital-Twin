//
// Created by Moritz Herzog on 25.04.24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_DigitalTwinMainWindow.h" resolved

#include "DigitalTwinMainWindow.h"
#include "../../ui/ui_DigitalTwinMainWindow.h"
#include "../Dialogs/SettingsDialog.h"

namespace DigitalTwin::Client {
    DigitalTwinMainWindow::DigitalTwinMainWindow(QWidget *parent) :
            QMainWindow(parent), ui(new Ui::DigitalTwinMainWindow) {
        ui->setupUi(this);

        makeConnecitons();
    }

    DigitalTwinMainWindow::~DigitalTwinMainWindow() {
        delete ui;
    }

    void DigitalTwinMainWindow::makeConnecitons() {
        connect(ui->ProjectsDockWidget, SIGNAL(visibilityChanged(bool)), this, SLOT(toggleButtonCheckBoxProjects(bool)));
        connect(ui->VariablesDockWidget, SIGNAL(visibilityChanged(bool)), this, SLOT(toggleButtonCheckBoxVariables(bool)));
        connect(ui->actionShow_Variables_Dockwidget, SIGNAL(toggled(bool)), this, SLOT(showVariablesDockWidget(bool)));
        connect(ui->actionShow_Projects_Dockwidget, SIGNAL(toggled(bool)), this, SLOT(showProjectDockWidget(bool)));
        connect(ui->actionConnect_to_Remotes,&QAction::triggered, this, &DigitalTwinMainWindow::showSettingsDialog);
    }

    void DigitalTwinMainWindow::toggleButtonCheckBoxProjects(bool visibility) {
        ui->actionShow_Projects_Dockwidget->setChecked(visibility);
    }

    void DigitalTwinMainWindow::toggleButtonCheckBoxVariables(bool visibility) {
        ui->actionShow_Variables_Dockwidget->setChecked(visibility);
    }

    void DigitalTwinMainWindow::showVariablesDockWidget(bool visibility) {
        ui->VariablesDockWidget->setVisible(visibility);

    }

    void DigitalTwinMainWindow::showProjectDockWidget(bool visibility) {
        ui->ProjectsDockWidget->setVisible(visibility);
    }

    void DigitalTwinMainWindow::showSettingsDialog() {
        SettingsDialog dialog = SettingsDialog();
        dialog.exec();
    }

} // DigitalTwin::Client
