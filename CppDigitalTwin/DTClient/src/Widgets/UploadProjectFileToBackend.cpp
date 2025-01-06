//
// Created by Moritz Herzog on 03.01.25.
//

#include "UploadProjectFileToBackend.h"
#include "ui_UploadProjectFileToBackend.h"


namespace DigitalTwin::Client {
    UploadProjectFileToBackend::UploadProjectFileToBackend(QWidget *parent) :
            QMainWindow(parent),
            Ui(new Ui::UploadProjectFileToBackend()),
			Parser(new MarkdownParser())
    {
        Ui->setupUi(this);
        Ui->retranslateUi(this);

        makeConnections();
    }

    UploadProjectFileToBackend::~UploadProjectFileToBackend() {
        delete Ui;
    }

    void UploadProjectFileToBackend::setHTMLTextForView(QString htmlText)
    {
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
    }

    void UploadProjectFileToBackend::setMarkdownOfOnlineProject(QString Markdown) {
        Parser->parseMarkdown(Markdown);
        Ui->MarkdownViewer->setHtml(Parser->getHTMLOfMarkdown());
        this->setWindowModified(true);
    }
}
