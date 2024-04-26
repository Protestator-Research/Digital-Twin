//
// Created by Moritz Herzog on 25.04.24.
//

#ifndef DIGITALTWIN_DIGITALTWINMAINWINDOW_H
#define DIGITALTWIN_DIGITALTWINMAINWINDOW_H

#include <QMainWindow>

//---------------------------------------------------
//Forwarding
//---------------------------------------------------
QT_BEGIN_NAMESPACE
namespace DigitalTwin::Client::Ui { class DigitalTwinMainWindow; }
QT_END_NAMESPACE

namespace DigitalTwin::Client {
    class DigitalTwinMainWindow : public QMainWindow {
    Q_OBJECT

    public:
        explicit DigitalTwinMainWindow(QWidget *parent = nullptr);

        ~DigitalTwinMainWindow() override;

    private:
        Ui::DigitalTwinMainWindow *ui;

        void makeConnecitons();

    private slots:
         void toggleButtonCheckBoxProjects(bool visibility);
         void toggleButtonCheckBoxVariables(bool visibility);
         void showVariablesDockWidget(bool visibility);
         void showProjectDockWidget(bool visibility);
         void showSettingsDialog();

    };
} // DigitalTwin::Client

#endif //DIGITALTWIN_DIGITALTWINMAINWINDOW_H
