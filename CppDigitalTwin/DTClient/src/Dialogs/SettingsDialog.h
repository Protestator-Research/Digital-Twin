//
// Created by Moritz Herzog on 25.04.24.
//

#ifndef DIGITALTWIN_SETTINGSDIALOG_H
#define DIGITALTWIN_SETTINGSDIALOG_H

#include <QDialog>

namespace DigitalTwin::Client {
    QT_BEGIN_NAMESPACE
    namespace Ui { class SettingsDialog; }
    QT_END_NAMESPACE

    class SettingsDialog : public QDialog {
    Q_OBJECT

    public:
        explicit SettingsDialog(QWidget *parent = nullptr);

        ~SettingsDialog() override;

    private:
        Ui::SettingsDialog *ui;
    };
} // DigitalTwin::Client

#endif //DIGITALTWIN_SETTINGSDIALOG_H
