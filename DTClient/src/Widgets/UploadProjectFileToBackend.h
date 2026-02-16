//
// Created by Moritz Herzog on 03.01.25.
//

#ifndef DIGITALTWIN_UPLOADPROJECTFILETOBACKEND_H
#define DIGITALTWIN_UPLOADPROJECTFILETOBACKEND_H

#include <QMainWindow>
#include <QStandardItemModel>
#include "../Models/Markdown/MarkdownParser.h"
#include <vector>
#include <kerml/root/elements/Element.h>
#include <sysmlv2/rest/entities/Commit.h>
#include <sysmlv2/rest/entities/Project.h>
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

        void setElementsForView(std::vector<std::shared_ptr<KerML::Entities::Element>> elements, std::shared_ptr<SysMLv2::REST::Commit> commit, std::shared_ptr<SysMLv2::REST::Project> project);
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

        std::vector<std::shared_ptr<KerML::Entities::Element>> Elements;
        Ui::UploadProjectFileToBackend* Ui;
        MarkdownParser* Parser;
        QStandardItemModel* DTElementsModels;
        BACKEND_COMMUNICATION::CommunicationService* CommunicationService;

        std::shared_ptr<SysMLv2::REST::Commit> Commit;
        std::shared_ptr<SysMLv2::REST::Project> Project;

        UploadProjectFileToBackendStatus Status;

    };
}

#endif //DIGITALTWIN_UPLOADPROJECTFILETOBACKEND_H
