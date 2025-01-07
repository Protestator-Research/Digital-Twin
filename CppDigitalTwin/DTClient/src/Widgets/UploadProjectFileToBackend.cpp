//
// Created by Moritz Herzog on 03.01.25.
//

#include "UploadProjectFileToBackend.h"
#include "ui_UploadProjectFileToBackend.h"
#include "Dialogs/CreateProjectDialog.h"

#include <QToolBar>
#include <QMessageBox>

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
        Ui->listView->setModel(DTElementsModels);
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
        setCodeElements();
        QString markdown;
        for(const auto elem : elements){
            markdown += QString::fromStdString(elem->getMarkdownString());
        }
        setMarkdownOfOnlineProject(markdown);
    }

    void UploadProjectFileToBackend::setCodeElements() {
        for(const auto& elem : Elements) {
            if(!elem->body().empty())
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
            Parser->getElementsOfProject();

        }
    }


}
