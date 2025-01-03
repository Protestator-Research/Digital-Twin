//
// Created by Moritz Herzog on 03.01.25.
//

#ifndef DIGITALTWIN_UPLOADPROJECTFILETOBACKEND_H
#define DIGITALTWIN_UPLOADPROJECTFILETOBACKEND_H

#include <QMainWindow>

namespace DigitalTwin::Client {
    namespace Ui {
        class UploadProjectFileToBackend;
    }
}

namespace DigitalTwin::Client {
    class UploadProjectFileToBackend : public QMainWindow {
    public:
        explicit UploadProjectFileToBackend(QWidget* parent = NULL);
        ~UploadProjectFileToBackend();

    private:
        Ui::UploadProjectFileToBackend* Ui;
    };
}

#endif //DIGITALTWIN_UPLOADPROJECTFILETOBACKEND_H
