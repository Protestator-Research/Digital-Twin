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
        // The MQTT connection first: it stops the Paho threads, whose callbacks use the manager.
        delete ConnectionThread;
        ConnectionThread = nullptr;
        delete DigitalTwinManager;
        DigitalTwinManager = nullptr;
        delete BackendCommunication;
        BackendCommunication = nullptr;
        delete Settings;
        delete ProjectViewModel;
    }

    void MainWindowModel::connectToBackend() {
        // The server refused the login: build a new connection with the (possibly corrected) settings. The manager
        // stays, the opened tabs keep their models; it is switched to the new connection below and the refused one
        // is deleted only afterwards, so the manager never points to a deleted connection.
        MQTTConnectionThread* refusedConnection = nullptr;
        if(ConnectionThread != nullptr && ConnectionThread->isLoginRefused()) {
            refusedConnection = ConnectionThread;
            ConnectionThread = nullptr;
        }
        // Already connected: a second connection would use the same MQTT client id and leak the first one.
        if(ConnectionThread != nullptr && DigitalTwinManager != nullptr) {
            qDebug()<<"MainWindowModel::connectToBackend(): already connected";
            return;
        }
        try {
            if(BackendCommunication == nullptr)
                BackendCommunication=new BACKEND_COMMUNICATION::CommunicationService(Settings->getRESTServerAsString(),std::stoi(Settings->getRESTPortAsString()), Settings->getRESTFolderAsString());
            if(ConnectionThread == nullptr) {
                // The DT server checks the MQTT login at its backend; it is the REST login or an own MQTT login, as set in the settings.
                ConnectionThread = new MQTTConnectionThread(Settings->getMQTTServerAsString(), Settings->getMQTTPortAsString(), Settings->getMQTTProtocolVersion(),
                                                            Settings->getMQTTLoginUserAsString(), Settings->getMQTTLoginPasswordAsString());
                connect(ConnectionThread, &MQTTConnectionThread::loginRefused, this, [this]() {
                    QMessageBox::warning(MainWindow, tr("Login at the DT server failed"),
                                         tr("The DT server refused the user name or password of the MQTT login. "
                                            "The MQTT connection is not retried. Please check the settings and connect again."));
                });
                if(DigitalTwinManager == nullptr)
                    DigitalTwinManager = new DigitalTwin::DigitalTwinManager(BackendCommunication, ConnectionThread);
                else
                    DigitalTwinManager->setClientService(ConnectionThread);
                // MQTT callbacks arrive on Paho threads; models are changed in the thread of the connection object (GUI).
                MQTTConnectionThread* connectionThread = ConnectionThread;
                DigitalTwinManager->setCallbackDispatcher([connectionThread](std::function<void()> work) {
                    QMetaObject::invokeMethod(connectionThread, std::move(work), Qt::QueuedConnection);
                });
                // After every (re)connect: announce the opened twins again (requests made while not connected are
                // dropped, and a restarted server has forgotten them). Delivered in the GUI thread.
                connect(ConnectionThread, &MQTTConnectionThread::connected, this, [this]() {
                    if(DigitalTwinManager != nullptr)
                        DigitalTwinManager->announceOpenTwins();
                });
                ConnectionThread->start();
                // Waits for its Paho callbacks; queued work for it is dropped with the object.
                delete refusedConnection;
                refusedConnection = nullptr;
            }
            BackendCommunication->setUserForLoginInBackend(Settings->getRESTUserAsString(),Settings->getRESTPasswordAsString());
            Status = MainWindowStatus::CONNECTED;

            refreshProjects();

        }catch (std::exception &ex){
            // The new connection could not be built: keep the refused one, the manager still uses it.
            if(refusedConnection != nullptr) {
                if(ConnectionThread == nullptr)
                    ConnectionThread = refusedConnection;
                else if(DigitalTwinManager != nullptr) {
                    // The new connection exists: make sure the manager uses it before the refused one goes away.
                    DigitalTwinManager->setClientService(ConnectionThread);
                    delete refusedConnection;
                }
            }
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
