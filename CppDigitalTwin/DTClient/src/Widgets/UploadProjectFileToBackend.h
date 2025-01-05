//
// Created by Moritz Herzog on 03.01.25.
//

#ifndef DIGITALTWIN_UPLOADPROJECTFILETOBACKEND_H
#define DIGITALTWIN_UPLOADPROJECTFILETOBACKEND_H

#include <QMainWindow>
#include "../Models/Markdown/MarkdownParser.h"

namespace DigitalTwin::Client {
    namespace Ui {
        class UploadProjectFileToBackend;
    }
}

namespace DigitalTwin::Client {
    class UploadProjectFileToBackend : public QMainWindow {
        Q_OBJECT
    public:
        explicit UploadProjectFileToBackend(QWidget* parent = NULL);
        ~UploadProjectFileToBackend() override;

        void setHTMLTextForView(QString htmlText);

    private slots:
        void onTextEdited();

    private:
        void makeConnections();

        Ui::UploadProjectFileToBackend* Ui;
        MarkdownParser* Parser;
    };
}

#endif //DIGITALTWIN_UPLOADPROJECTFILETOBACKEND_H
