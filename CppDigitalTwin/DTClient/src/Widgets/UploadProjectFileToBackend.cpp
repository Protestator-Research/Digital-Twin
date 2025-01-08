//
// Created by Moritz Herzog on 03.01.25.
//

#include "UploadProjectFileToBackend.h"
#include "ui_UploadProjectFileToBackend.h"
#include "Dialogs/CreateProjectDialog.h"

#include <QToolBar>
#include <QMessageBox>
#include <iostream>

#include <SysMLv2Standard/entities/Commit.h>
#include <SysMLv2Standard/entities/DataVersion.h>
#include <boost/uuid/random_generator.hpp>

#include "SysMLv2Standard/entities/DataIdentity.h"
#include "SysMLv2Standard/entities/Project.h"

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
        Ui->MarkdownViewer->setHtml(Parser->getHTMLOfMarkdown());
        Ui->ProjektEditor->setPlainText(Parser->getMarkdownString());
        this->setWindowModified(false);
        redecorateWithStatusChange();
    }

    void UploadProjectFileToBackend::onTextEdited()
    {
        Parser->parseMarkdown(Ui->ProjektEditor->toPlainText());
        Ui->MarkdownViewer->setHtml(Parser->getHTMLOfMarkdown());
        this->setWindowModified(true);
    }

    void UploadProjectFileToBackend::makeConnections()
    {
        connect(Ui->ProjektEditor, SIGNAL(textChanged()), this, SLOT(onTextEdited()));
        connect(Ui->actionUpload, SIGNAL(triggered(bool)), this, SLOT(onActionUploadClicked()));
        connect(Ui->actionDownload, SIGNAL(triggered(bool)), this, SLOT(onActionDownloadClicked()));
        connect(Ui->CreateDTButton, SIGNAL(clicked(bool)), this, SLOT(onCreateDigitalTwinClicked()));
    }

    void UploadProjectFileToBackend::setMarkdownOfOnlineProject(QString Markdown) {
        Ui->ProjektEditor->setText(Markdown);
        Parser->parseMarkdown(Markdown);
        Ui->MarkdownViewer->setHtml(Parser->getHTMLOfMarkdown());
        this->setWindowModified(false);
    }

    void UploadProjectFileToBackend::setElementsForView(std::vector<SysMLv2::Entities::Element *> elements) {
        Status = UploadProjectFileToBackendStatus::OnlineProjectOpened;
        Elements = elements;
        redecorateWithStatusChange();
    }

    void UploadProjectFileToBackend::setCodeElements() {
        for(const auto& elem : Elements) {
            if(!elem->body().empty() && (elem->language()!="Markdown"))
                DTElementsModels->appendRow( new QStandardItem(QString::fromStdString(elem->body())));
        }
    }

    void UploadProjectFileToBackend::onActionUploadClicked() {
        if (Status==UploadProjectFileToBackendStatus::DocumentOpened) {
            createOnlineProject();
        }
    }

    void UploadProjectFileToBackend::onActionDownloadClicked() {

    }

    void UploadProjectFileToBackend::createOnlineProject() {
        CreateProjectDialog dialog(this);
        dialog.exec();
        if(dialog.result()==QDialog::DialogCode::Accepted) {
            auto project = CommunicationService->postProject(dialog.getProjectName(), dialog.getProjectDecription(), "Main");
            Elements = Parser->getElementsOfProject();
            auto commit = new SysMLv2::Entities::Commit(dialog.getProjectName(), dialog.getProjectDecription(), project);

            std::vector<SysMLv2::Entities::DataVersion*> dataVersions;
            for (const auto& element : Elements)
            {
                auto dataVersion = new SysMLv2::Entities::DataVersion(new SysMLv2::Entities::DataIdentity(boost::uuids::random_generator()()), element);
                dataVersions.push_back(dataVersion);
            }

            commit->setChange(dataVersions);
            CommunicationService->postCommitWithId(project->getId(), commit);

            Status=UploadProjectFileToBackendStatus::OnlineProjectOpened;
            redecorateWithStatusChange();
            this->setWindowModified(false);
        }
    }

    void UploadProjectFileToBackend::redecorateWithStatusChange() {
        if(Status==UploadProjectFileToBackendStatus::OnlineProjectOpened) {
            setCodeElements();
            QString markdown;
            for(const auto elem : Elements){
                markdown += QString::fromStdString(elem->getMarkdownString());
            }
            setMarkdownOfOnlineProject(markdown);
            Ui->actionDownload->setDisabled(false);
            Ui->actionUpload->setDisabled(true);
            Ui->DTTab->setDisabled(false);
        }else {
            Ui->actionUpload->setDisabled(false);
            Ui->actionDownload->setDisabled(true);
            Ui->DTTab->setDisabled(true);
        }

    }

    void UploadProjectFileToBackend::onCreateDigitalTwinClicked() {
        std::vector<SysMLv2::Entities::Element *> elements;
        for(const auto& elem : Elements) {
            if(!elem->body().empty() && (elem->language()!="Markdown"))
                elements.push_back( elem);
        }

        std::vector<SysMLv2::Entities::Element *> selectedElements;

        const auto selectedItems = Ui->DTModelListView->selectionModel()->selectedRows();
        for(const auto & element : selectedItems) {
            selectedElements.push_back(elements[element.row()]);
        }


    }


}
