//
// Created by Moritz Herzog on 03.01.25.
//

#include "UploadProjectFileToBackend.h"
#include "ui_UploadProjectFileToBackend.h"
#include "Dialogs/CreateProjectDialog.h"

#include <QToolBar>
#include <QMessageBox>
#include <iostream>

#include <sysmlv2/rest/entities/DataVersion.h>
#include <boost/uuid/random_generator.hpp>
#include <sysmlv2/rest/entities/DataIdentity.h>
#include <sysmlv2/rest/entities/Commit.h>
#include <sysmlv2/rest/entities/CommitRequest.h>
#include <kerml/root/elements/Element.h>
#include <kerml/root/annotations/TextualRepresentation.h>
#include <memory>

#include "entities/TwinRequest.h"
#include "Model/ModelBlocks.h"
#include <stdexcept>
#include <QStatusBar>
//#include "AGILABackendImplementation/DigitalTwin.h"

namespace DigitalTwin::Client {
    UploadProjectFileToBackend::UploadProjectFileToBackend(BACKEND_COMMUNICATION::CommunicationService* service, QWidget *parent) :
            QMainWindow(parent),
            Ui(new Ui::UploadProjectFileToBackend()),
			Parser(new MarkdownParser()),
            DTElementsModels(new QStandardItemModel()),
            CommunicationService(service)
    {
        Ui->setupUi(this);
        Ui->retranslateUi(this);

        QStringList headerElements;
        headerElements << tr("Code");

        DTElementsModels->setHorizontalHeaderLabels(headerElements);
        Ui->DTModelListView->setModel(DTElementsModels);
        auto toolbar = new QToolBar(this);
        addToolBar(toolbar);

        Ui->actionDownload->setIcon(QIcon(":/icons/Download"));
        toolbar->addAction(Ui->actionDownload);
        Ui->actionUpload->setIcon(QIcon(":/icons/Upload"));
        toolbar->addAction(Ui->actionUpload);

        makeConnections();
    }

    UploadProjectFileToBackend::~UploadProjectFileToBackend() {
        delete Ui;
    }

    void UploadProjectFileToBackend::setHTMLTextForView(QString htmlText)
    {
        Status = UploadProjectFileToBackendStatus::DocumentOpened;
        Parser->parseMarkdownFile(htmlText);
        this->setWindowModified(false);
        redecorateWithStatusChange();
    }

    void UploadProjectFileToBackend::onTextEdited()
    {
        this->setWindowModified(true);
    }

    void UploadProjectFileToBackend::makeConnections()
    {
        connect(Ui->CreateDTButton, SIGNAL(clicked(bool)), this, SLOT(onCreateDigitalTwinClicked()));
    }

    void UploadProjectFileToBackend::setMarkdownOfOnlineProject(QString Markdown) {
        Parser->parseMarkdown(Markdown);
        this->setWindowModified(false);
    }

    void UploadProjectFileToBackend::setElementsForView(std::vector<std::shared_ptr<KerML::Entities::Element>> elements, std::shared_ptr<SysMLv2::REST::Commit> commit, std::shared_ptr<SysMLv2::REST::Project> project) {
        Status = UploadProjectFileToBackendStatus::OnlineProjectOpened;
        Elements = elements;
        Commit = commit;
        Project = project;
        redecorateWithStatusChange();
    }

    void UploadProjectFileToBackend::setCodeElements() {
        DTElementsModels->removeRows(0, DTElementsModels->rowCount());
        // The same selection and order as in onCreateDigitalTwinClicked and in the digital twin model.
        for(const auto& block : DigitalTwin::Model::codeBlocksInOrder(Elements))
            DTElementsModels->appendRow( new QStandardItem(QString::fromStdString(block->body())));
    }

    void UploadProjectFileToBackend::createOnlineProject() {
        const bool retryOnlyCommit = (Project != nullptr && Commit == nullptr && PendingCommitRequest != nullptr);

        if(!retryOnlyCommit) {
            CreateProjectDialog dialog(this);
            dialog.exec();
            if(dialog.result()!=QDialog::DialogCode::Accepted)
                return;

            try {
                const auto blocks = Parser->getElementsOfProject();
                if(blocks.empty()) {
                    QMessageBox::information(this, tr("Empty document"),
                                             tr("The document has no content. The project is not created, because the server rejects an empty commit."));
                    return;
                }

                const std::string projectName = dialog.getProjectName();
                // Built before the project is created, so that a document that cannot be committed creates no project.
                // The first commit holds a root package with the blocks in document order, see Model/ModelBlocks.h.
                auto documentCommit = DigitalTwin::Model::createDocumentCommit(projectName, blocks);
                Project = CommunicationService->postProject(projectName, dialog.getProjectDecription(), "main");
                PendingCommitRequest = documentCommit.Request;
            } catch(const std::exception& ex) {
                QMessageBox::critical(this, tr("Creating the project failed"),
                                      QString::fromStdString(BACKEND_COMMUNICATION::CommunicationService::describeBackendError(ex)));
                return;
            }
        } else {
            const auto answer = QMessageBox::question(this, tr("Project without content"),
                    tr("The project \"%1\" exists on the server, but its content was not uploaded. Upload the content now? "
                       "No second project is created.").arg(QString::fromStdString(Project->getName())));
            if(answer != QMessageBox::Yes)
                return;
        }

        try {
            Commit = CommunicationService->postCommitWithId(Project->getId(), PendingCommitRequest);
            PendingCommitRequest = nullptr;
        } catch(const std::exception& ex) {
            // The project stays, the next call only repeats the commit.
            QMessageBox::critical(this, tr("Uploading the content failed"),
                                  tr("The project \"%1\" was created on the server, but it has no content yet. "
                                     "Start the upload again to repeat only the content.\n\n%2")
                                  .arg(QString::fromStdString(Project->getName()),
                                       QString::fromStdString(BACKEND_COMMUNICATION::CommunicationService::describeBackendError(ex))));
            return;
        }

        try {
            // The elements come from the server: only they are typed (TextualRepresentation) and complete.
            Elements = CommunicationService->getAllElementsOfCommit(Project->getId(), Commit->getId());
        } catch(const std::exception& ex) {
            Elements.clear();
            QMessageBox::warning(this, tr("Loading the project failed"),
                                 QString::fromStdString(BACKEND_COMMUNICATION::CommunicationService::describeBackendError(ex)));
        }

        Status=UploadProjectFileToBackendStatus::OnlineProjectOpened;
        redecorateWithStatusChange();
        this->setWindowModified(false);
    }

    void UploadProjectFileToBackend::redecorateWithStatusChange() {
        if(Status==UploadProjectFileToBackendStatus::OnlineProjectOpened) {
            setCodeElements();
            QString markdown;
            for([[maybe_unused]] const auto& elem : Elements){
                //markdown += QString::fromStdString(elem->getMarkdownString());
            }
            setMarkdownOfOnlineProject(markdown);
            Ui->actionDownload->setDisabled(false);
            Ui->actionUpload->setDisabled(true);
            Ui->DTTab->setDisabled(false);
            // A project without commit has nothing a digital twin could reference.
            const bool hasCommit = (Commit != nullptr);
            Ui->CreateDTButton->setEnabled(hasCommit);
            Ui->CreateDTButton->setToolTip(hasCommit ? QString() : tr("The project has no commit yet."));
            if(!hasCommit)
                statusBar()->showMessage(tr("The project has no commit yet, a digital twin cannot be created."));
        }else {
            Ui->actionUpload->setDisabled(false);
            Ui->actionDownload->setDisabled(true);
            Ui->DTTab->setDisabled(true);
        }

    }

    void UploadProjectFileToBackend::onCreateDigitalTwinClicked() {
        if(Commit == nullptr || Project == nullptr) {
            QMessageBox::information(this, tr("No commit"), tr("The project has no commit yet, a digital twin cannot be created."));
            return;
        }

        // Same list as in setCodeElements, thus the row of the view is the index in this list.
        const auto blocks = DigitalTwin::Model::codeBlocksInOrder(Elements);

        std::vector<boost::uuids::uuid> selectedElements;

        const auto selectedItems = Ui->DTModelListView->selectionModel()->selectedRows();
        for(const auto & element : selectedItems) {
            if(element.row() >= 0 && static_cast<std::size_t>(element.row()) < blocks.size())
                selectedElements.push_back(blocks[static_cast<std::size_t>(element.row())]->getId());
        }

        try {
            std::shared_ptr<SysMLv2::REST::TwinRequest> twinRequest = std::make_shared<SysMLv2::REST::TwinRequest>(Ui->DTNameLineEdit->text().toStdString(), Commit);
            CommunicationService->postDigitalTwin(twinRequest, Project->getId());
        } catch(const std::exception& ex) {
            QMessageBox::critical(this, tr("Creating the digital twin failed"),
                                  QString::fromStdString(BACKEND_COMMUNICATION::CommunicationService::describeBackendError(ex)));
        }
    }


}
