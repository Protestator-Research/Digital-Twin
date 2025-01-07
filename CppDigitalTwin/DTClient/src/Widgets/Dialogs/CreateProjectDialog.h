//
// Created by Moritz Herzog on 07.01.25.
//

#ifndef DIGITALTWIN_CREATEPROJECTDIALOG_H
#define DIGITALTWIN_CREATEPROJECTDIALOG_H

#include <QDialog>

namespace DigitalTwin::Client {
    QT_BEGIN_NAMESPACE
    namespace Ui {
        class CreateProjectDialog;
    }
    QT_END_NAMESPACE

    class CreateProjectDialog : public QDialog {
    Q_OBJECT

    public:
        explicit CreateProjectDialog(QWidget *parent = nullptr);

        ~CreateProjectDialog() override;

        std::string getProjectName();
        std::string getProjectDecription();
    private:
        Ui::CreateProjectDialog *Ui;
    };
} // DigitalTwin::Client

#endif //DIGITALTWIN_CREATEPROJECTDIALOG_H
