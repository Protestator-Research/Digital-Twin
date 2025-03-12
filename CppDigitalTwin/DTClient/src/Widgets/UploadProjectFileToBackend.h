//
// Created by Moritz Herzog on 03.01.25.
//

#ifndef DIGITALTWIN_UPLOADPROJECTFILETOBACKEND_H
#define DIGITALTWIN_UPLOADPROJECTFILETOBACKEND_H

#include <QMainWindow>
#include <QStandardItemModel>
#include "../Models/Markdown/MarkdownParser.h"
#include <vector>
#include <sysmlv2/entities/Element.h>
#include <sysmlv2/entities/Commit.h>
#include <sysmlv2/entities/Project.h>
#include <BECommunicationService.h>

namespace DigitalTwin::Client {
    namespace Ui {
        class UploadProjectFileToBackend;
    }
}

namespace DigitalTwin::Client {
    class UploadProjectFileToBackend : public QMainWindow {
        Q_OBJECT

        enum UploadProjectFileToBackendStatus {
            DocumentOpened,
            OnlineProjectOpened
        };

    public:
        explicit UploadProjectFileToBackend(BACKEND_COMMUNICATION::CommunicationService* service, QWidget* parent = NULL);
        ~UploadProjectFileToBackend() override;

        void setElementsForView(std::vector<SysMLv2::Entities::Element*> elements, SysMLv2::Entities::Commit* commit, SysMLv2::Entities::Project* project);
        void setHTMLTextForView(QString htmlText);

    private slots:
        void onTextEdited();
        void onActionUploadClicked();
        void onActionDownloadClicked();
        void onCreateDigitalTwinClicked();

    private:
        void makeConnections();
        void setMarkdownOfOnlineProject(QString Markdown);
        void setCodeElements();
        void createOnlineProject();
        void redecorateWithStatusChange();

        std::vector<SysMLv2::Entities::Element *> Elements;
        Ui::UploadProjectFileToBackend* Ui;
        MarkdownParser* Parser;
        QStandardItemModel* DTElementsModels;
        BACKEND_COMMUNICATION::CommunicationService* CommunicationService;

        SysMLv2::Entities::Commit* Commit;
        SysMLv2::Entities::Project* Project;

        UploadProjectFileToBackendStatus Status;

    };
}

#endif //DIGITALTWIN_UPLOADPROJECTFILETOBACKEND_H
