//
// Created by Moritz Herzog on 14.05.24.
//

#include <QList>
#include <BECommunicationService.h>
#include <string>
#include <vector>
#include <SysMLv2Standard/entities/Project.h>
#include <SysMLv2Standard/entities/DigitalTwin.h>

#include "MainWindowModel.h"
#include "DigitalTwinClientSettings.h"
#include "../Widgets/DigitalTwinMainWindow.h"
#include "../Widgets/TreeViewModels/ProjectTreeViewModel.h"


namespace DigitalTwin::Client {
    MainWindowModel::MainWindowModel(DigitalTwinMainWindow *mainWindow) {
        MainWindow = mainWindow;
        Settings = new DigitalTwinClientSettings(MainWindow);
        Status = MainWindowStatus::NOT_CONNECTED;
        decorateTreeView();
    }

    MainWindowModel::~MainWindowModel() {
        delete Settings;
        delete ProjectViewModel;
        if(BackendCommunication!= nullptr)
            delete BackendCommunication;
    }

    void MainWindowModel::connectToBackend() {
        try {
            BackendCommunication=new BACKEND_COMMUNICATION::CommunicationService(Settings->getRESTServerAsString(),std::stoi(Settings->getRESTPortAsString()));
            BackendCommunication->setUserForLoginInBackend(Settings->getRESTUserAsString(),Settings->getRESTPasswordAsString());
            Status = MainWindowStatus::CONNECTED;

            Projects = BackendCommunication->getAllProjects();
            ProjectViewModel->setProjects(Projects);

            for(auto project : Projects) {
                auto digitalTwins = BackendCommunication->getAllDigitalTwinsForProjectWithId(project->getId());
                DigitalTwinMap.emplace(project->getId(),digitalTwins);
                ProjectViewModel->setDigitalTwinForProjectWithId(project, digitalTwins);
            }

        }catch (std::exception &ex){
            qDebug()<<ex.what();
        }
    }

    DigitalTwinClientSettings *MainWindowModel::clientSettings() const {
        return Settings;
    }

    void MainWindowModel::decorateTreeView() {
        ProjectViewModel = new ViewModels::ProjectTreeViewModel(MainWindow);
        MainWindow->setProjectTreeViewModel(ProjectViewModel);
    }

    void MainWindowModel::onTreeViewClicked(const QModelIndex &index) {
        auto item = ProjectViewModel->getProjectTreeViewItemFromIndex(index);
        auto possibleDigitalTwin = item->getDigitalTwin();
        if(possibleDigitalTwin!= nullptr){
            qDebug()<<possibleDigitalTwin->getName();
        }
        qDebug()<<index;
    }
}
