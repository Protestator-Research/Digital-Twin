//
// Created by Moritz Herzog on 14.05.24.
//

#ifndef DIGITALTWIN_MAINWINDOWMODEL_H
#define DIGITALTWIN_MAINWINDOWMODEL_H

#include "MainWindowStatus.h"
#include <map>
#include <vector>
#include <boost/uuid/uuid.hpp>
#include <QObject>

namespace DigitalTwin::Client{
    class DigitalTwinMainWindow;
    class DigitalTwinClientSettings;
    class MQTTConnectionThread;
    namespace ViewModels {
        class ProjectTreeViewModel;
    }
}

namespace BACKEND_COMMUNICATION{
    class CommunicationService;
}

namespace SysMLv2::Entities{
    class DigitalTwin;
    class Project;
}

namespace DigitalTwin {
    class DigitalTwinManager;
}

namespace PHYSICAL_TWIN_COMMUNICATION {
    class MqttClientService;
}

namespace DigitalTwin::Client {
    /**
     *
     */
    class MainWindowModel : public QObject{
        Q_OBJECT
    public:
        MainWindowModel() = delete;
        explicit MainWindowModel(DigitalTwinMainWindow* mainWindow);
        virtual ~MainWindowModel() override;

        void connectToBackend();
        void connectToDigitalTwin();
        [[nodiscard]] DigitalTwinClientSettings* clientSettings() const;

        void openMarkdownFile(QString filePath);

        bool isOnline();

        void refreshProjects();

    public slots:
        void onTreeViewClicked(const QModelIndex& index);

    private:
        void decorateTreeView();

        DigitalTwinMainWindow* MainWindow;
        DigitalTwinClientSettings* Settings;
        MainWindowStatus Status;
        ViewModels::ProjectTreeViewModel *ProjectViewModel;
        BACKEND_COMMUNICATION::CommunicationService *BackendCommunication = nullptr;
        MQTTConnectionThread* ConnectionThread = nullptr;
        std::map<boost::uuids::uuid,std::vector<SysMLv2::Entities::DigitalTwin*>> DigitalTwinMap;
        DigitalTwin::DigitalTwinManager* DigitalTwinManager = nullptr;
        std::vector<std::shared_ptr<SysMLv2::Entities::Project>> Projects;
    };
}

#endif //DIGITALTWIN_MAINWINDOWMODEL_H
