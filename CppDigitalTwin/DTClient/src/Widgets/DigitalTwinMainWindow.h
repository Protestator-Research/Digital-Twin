//
// Created by Moritz Herzog on 25.04.24.
//

#ifndef DIGITALTWIN_DIGITALTWINMAINWINDOW_H
#define DIGITALTWIN_DIGITALTWINMAINWINDOW_H

#include <QMainWindow>
#include <QToolBar>
#include <QAbstractItemModel>

//---------------------------------------------------
//Forwarding
//---------------------------------------------------
QT_BEGIN_NAMESPACE
namespace DigitalTwin::Client {

    class MainWindowModel;

    namespace Ui {
        class DigitalTwinMainWindow;
    }
}

namespace BACKEND_COMMUNICATION {
    class CommunicationService;
}
QT_END_NAMESPACE

namespace DigitalTwin::Client {
    class DigitalTwinMainWindow : public QMainWindow {
    Q_OBJECT

    public:
        explicit DigitalTwinMainWindow(QWidget *parent = nullptr);
        ~DigitalTwinMainWindow() override;

        void setProjectTreeViewModel(QAbstractItemModel* model);

    private:
        Ui::DigitalTwinMainWindow *ui;
        QToolBar *toolBar;
        MainWindowModel* Model;


        void makeConnections();

        void decorateView();

    private slots:
         void toggleButtonCheckBoxProjects(bool visibility);
         void toggleButtonCheckBoxVariables(bool visibility);
         void showVariablesDockWidget(bool visibility);
         void showProjectDockWidget(bool visibility);
         void showSettingsDialog();
         void connectToServer();
    };
} // DigitalTwin::Client

#endif //DIGITALTWIN_DIGITALTWINMAINWINDOW_H
