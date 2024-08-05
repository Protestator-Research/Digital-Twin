//
// Created by Moritz Herzog on 25.04.24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_DigitalTwinMainWindow.h" resolved

#include "DigitalTwinMainWindow.h"
#include "../../ui/ui_DigitalTwinMainWindow.h"
#include "Dialogs/SettingsDialog.h"
#include "../Models/MainWindowModel.h"

namespace DigitalTwin::Client {
    DigitalTwinMainWindow::DigitalTwinMainWindow(QWidget *parent) :
            QMainWindow(parent),
            ui(new Ui::DigitalTwinMainWindow),
            toolBar(new QToolBar(this)) {
        ui->setupUi(this);

        Model = new MainWindowModel(this);

        makeConnections();
        decorateView();
    }

    DigitalTwinMainWindow::~DigitalTwinMainWindow() {
        delete ui;
    }

    void DigitalTwinMainWindow::makeConnections() {
        connect(ui->ProjectsDockWidget, SIGNAL(visibilityChanged(bool)), this, SLOT(toggleButtonCheckBoxProjects(bool)));
        connect(ui->actionShow_Projects_Dockwidget, SIGNAL(toggled(bool)), this, SLOT(showProjectDockWidget(bool)));
        connect(ui->actionConnect, SIGNAL(triggered(bool)), this, SLOT(connectToServer()));
        connect(ui->actionConnect_to_Remotes,&QAction::triggered, this, &DigitalTwinMainWindow::showSettingsDialog);
        connect(ui->ProjectTreeView, SIGNAL(clicked(const QModelIndex &)), Model, SLOT(onTreeViewClicked(const QModelIndex &)));
    }

    void DigitalTwinMainWindow::toggleButtonCheckBoxProjects(bool visibility) {
        ui->actionShow_Projects_Dockwidget->setChecked(visibility);
    }

    void DigitalTwinMainWindow::toggleButtonCheckBoxVariables(bool visibility) {
        ui->actionShow_Variables_Dockwidget->setChecked(visibility);
    }

    void DigitalTwinMainWindow::showProjectDockWidget(bool visibility) {
        ui->ProjectsDockWidget->setVisible(visibility);
    }

    void DigitalTwinMainWindow::showSettingsDialog() {
        SettingsDialog dialog = SettingsDialog(Model->clientSettings(), this);
        dialog.exec();
    }

    void DigitalTwinMainWindow::decorateView() {
        addToolBar(toolBar);

        ui->actionConnect_to_Remotes->setIcon(QIcon(":/icons/Settings"));
        toolBar->addAction(ui->actionConnect_to_Remotes);

        ui->actionConnect->setIcon(QIcon(":/icons/Connect"));
        toolBar->addAction(ui->actionConnect);
    }

    void DigitalTwinMainWindow::connectToServer() {
        Model->connectToBackend();
    }

    void DigitalTwinMainWindow::setProjectTreeViewModel(QAbstractItemModel *model) {
        ui->ProjectTreeView->setModel(model);
    }

} // DigitalTwin::Client
