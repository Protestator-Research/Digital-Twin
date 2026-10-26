//
// Created by Moritz Herzog on 14.05.24.
//

#include <QList>
#include <QMessageBox>
#include <BECommunicationService.h>
#include <string>
#include <sysmlv2/rest/entities/Project.h>
//#include "AGILABackendImplementation/DigitalTwin.h"
#include <DigitalTwinManager.h>
#include <Services/MqttClientService.h>
#include <sysmlv2/rest/entities/Branch.h>
#include <sysmlv2/rest/entities/Commit.h>
#include <kerml/root/elements/Element.h>

#include "MainWindowModel.h"
#include "DigitalTwinClientSettings.h"
#include "../Widgets/DigitalTwinMainWindow.h"
#include "../Widgets/TreeViewModels/ProjectTreeViewModel.h"
#include "../Widgets/DigitalTwinTabWidget/DigitalTwinTabWidget.h"
#include "../Widgets/UploadProjectFileToBackend.h"
#include "Markdown/MarkdownParser.h"

#include "MqttConnectionThread.h"
#include "entities/DigitalTwin.h"


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
        if(ConnectionThread != nullptr)
            delete ConnectionThread;
    }

    void MainWindowModel::connectToBackend() {
        try {
            BackendCommunication=new BACKEND_COMMUNICATION::CommunicationService(Settings->getRESTServerAsString(),std::stoi(Settings->getRESTPortAsString()), Settings->getRESTFolderAsString());
            ConnectionThread = new MQTTConnectionThread(Settings->getMQTTServerAsString(), Settings->getMQTTPortAsString());
            ConnectionThread->start();
            DigitalTwinManager = new DigitalTwin::DigitalTwinManager(BackendCommunication, ConnectionThread);
            BackendCommunication->setUserForLoginInBackend(Settings->getRESTUserAsString(),Settings->getRESTPasswordAsString());
            Status = MainWindowStatus::CONNECTED;

            refreshProjects();

        }catch (std::exception &ex){
            qDebug()<<QString::fromStdString(BACKEND_COMMUNICATION::CommunicationService::describeBackendError(ex));
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
            try {
                auto model = DigitalTwinManager->addDigitalTwinAndCreateModel(possibleDigitalTwin);
                MainWindow->addTabWidget(new DigitalTwinTabWidget(model,MainWindow),QString::fromStdString(possibleDigitalTwin->getName()));
            } catch (const std::exception& ex) {
                QMessageBox::critical(MainWindow, tr("Opening the digital twin failed"),
                                      QString::fromStdString(BACKEND_COMMUNICATION::CommunicationService::describeBackendError(ex)));
            }
        } else if(item->getProject() != nullptr) {
            auto project = item->getProject();
            try {
                std::vector<std::shared_ptr<KerML::Entities::Element>> elements;
                std::shared_ptr<SysMLv2::REST::Commit> commit = nullptr;
                // A new project has no commit: the default branch has no head and there is nothing to download.
                const auto defaultBranch = project->getDefaultBranch();
                if (defaultBranch != nullptr) {
                    for (const auto& branch : BackendCommunication->getAllBranchesForProjectWithID(project->getId())) {
                        if (branch != nullptr && branch->getId() == defaultBranch->getId()) {
                            commit = branch->getHead();
                            break;
                        }
                    }
                }
                if (commit != nullptr)
                    elements = BackendCommunication->getAllElements(commit->getId(), project->getId());

                UploadProjectFileToBackend* uploadFileDialog = new UploadProjectFileToBackend(BackendCommunication, MainWindow);
                uploadFileDialog->setElementsForView(elements,commit,project);
                uploadFileDialog->show();
            } catch (const std::exception& ex) {
                QMessageBox::critical(MainWindow, tr("Opening the project failed"),
                                      QString::fromStdString(BACKEND_COMMUNICATION::CommunicationService::describeBackendError(ex)));
            }
        }
    }

    void MainWindowModel::connectToDigitalTwin() {

    }

    void MainWindowModel::refreshProjects() {
        Projects = BackendCommunication->getAllProjects();
        ProjectViewModel->clearAllElements();
        DigitalTwinMap.clear();
        ProjectViewModel->setProjects(Projects);
        for (const auto& project : Projects) {
            DigitalTwinMap[project->getId()] = BackendCommunication->getAllDigitalTwinsForProjectWithId(project->getId());
            ProjectViewModel->setDigitalTwinForProjectWithId(project, DigitalTwinMap[project->getId()]);
        }
    }
}
