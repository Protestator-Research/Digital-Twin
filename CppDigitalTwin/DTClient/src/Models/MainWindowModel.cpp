//
// Created by Moritz Herzog on 14.05.24.
//

#include <QList>
#include <BECommunicationService.h>
#include <string>
#include <SysMLv2Standard/entities/Project.h>
#include <SysMLv2Standard/entities/DigitalTwin.h>
#include <DigitalTwinManager.h>
#include <Services/MqttClientService.h>
#include <SysMLv2Standard/entities/Branch.h>

#include "MainWindowModel.h"
#include "DigitalTwinClientSettings.h"
#include "../Widgets/DigitalTwinMainWindow.h"
#include "../Widgets/TreeViewModels/ProjectTreeViewModel.h"
#include "../Widgets/DigitalTwinTabWidget/DigitalTwinTabWidget.h"
#include "../Widgets/UploadProjectFileToBackend.h"
#include "Markdown/MarkdownParser.h"
#include "SysMLv2Standard/entities/Commit.h"
#include "SysMLv2Standard/entities/Element.h"


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
        delete DigitalTwinManager;
        if(BackendCommunication != nullptr)
            delete BackendCommunication;
        if(DigitalTwinManager != nullptr)
            delete DigitalTwinManager;
    }

    void MainWindowModel::connectToBackend() {
        try {
            BackendCommunication=new BACKEND_COMMUNICATION::CommunicationService(Settings->getRESTServerAsString(),std::stoi(Settings->getRESTPortAsString()), Settings->getRESTFolderAsString());
            ClientService = new PHYSICAL_TWIN_COMMUNICATION::MqttClientService(Settings->getMQTTServerAsString(), Settings->getMQTTPortAsString());
            DigitalTwinManager = new DigitalTwin::DigitalTwinManager(BackendCommunication, ClientService);
            BackendCommunication->setUserForLoginInBackend(Settings->getRESTUserAsString(),Settings->getRESTPasswordAsString());
            Status = MainWindowStatus::CONNECTED;

            refreshProjects();

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

    void MainWindowModel::openMarkdownFile(QString filePath) {
        UploadProjectFileToBackend *uploadFileDialog = new UploadProjectFileToBackend(BackendCommunication, MainWindow);
        uploadFileDialog->setHTMLTextForView(filePath);
        uploadFileDialog->show();
    }

    bool MainWindowModel::isOnline()
    {
        return (BackendCommunication != nullptr);
    }

    void MainWindowModel::decorateTreeView() {
        ProjectViewModel = new ViewModels::ProjectTreeViewModel(MainWindow);
        MainWindow->setProjectTreeViewModel(ProjectViewModel);
    }

    void MainWindowModel::onTreeViewClicked(const QModelIndex &index) {
        auto item = ProjectViewModel->getProjectTreeViewItemFromIndex(index);

        auto possibleDigitalTwin = item->getDigitalTwin();
        if(possibleDigitalTwin != nullptr){
            auto model = DigitalTwinManager->addDigitalTwinAndCreateModel(possibleDigitalTwin);
            MainWindow->addTabWidget(new DigitalTwinTabWidget(model,MainWindow),QString::fromStdString(possibleDigitalTwin->getName()));
        } else if(item->getProject() != nullptr) {
            auto project = item->getProject();
            UploadProjectFileToBackend* uploadFileDialog = new UploadProjectFileToBackend(BackendCommunication, MainWindow);
            auto branches = BackendCommunication->getAllBranchesForProjectWithID(project->getId());
            std::vector<SysMLv2::Entities::Element*> elements;
            SysMLv2::Entities::Commit* commit = nullptr;
            for (const auto& branch : branches)
                if (branch->getName() == "Main") {
                    commit = branch->getHead();
                    elements = BackendCommunication->getAllElements(branch->getHead()->getId(), project->getId());
                }

            uploadFileDialog->setElementsForView(elements,commit,project);
            uploadFileDialog->show();
        }
    }

    void MainWindowModel::connectToDigitalTwin() {

    }

    void MainWindowModel::refreshProjects() {
        Projects = BackendCommunication->getAllProjects();
        ProjectViewModel->clearAllElements();
        ProjectViewModel->setProjects(Projects);
    }
}
