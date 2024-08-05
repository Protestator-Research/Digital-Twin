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

/**
 *
 */
namespace DigitalTwin::Client {
    class MainWindowModel : public QObject{
        Q_OBJECT
    public:
        MainWindowModel() = delete;
        explicit MainWindowModel(DigitalTwinMainWindow* mainWindow);
        ~MainWindowModel() override;

        void connectToBackend();
        [[nodiscard]] DigitalTwinClientSettings* clientSettings() const;

    public slots:
        void onTreeViewClicked(const QModelIndex& index);

    private:
        void decorateTreeView();

        DigitalTwinMainWindow* MainWindow;
        DigitalTwinClientSettings* Settings;
        MainWindowStatus Status;
        ViewModels::ProjectTreeViewModel *ProjectViewModel;
        BACKEND_COMMUNICATION::CommunicationService *BackendCommunication = nullptr;
        std::map<boost::uuids::uuid,std::vector<SysMLv2::Entities::DigitalTwin*>> DigitalTwinMap;
        std::vector<SysMLv2::Entities::Project*> Projects;
    };
}

#endif //DIGITALTWIN_MAINWINDOWMODEL_H
