//
// Created by Moritz Herzog on 23.05.24.
//

#ifndef DIGITALTWIN_DIGITALTWINTABWIDGET_H
#define DIGITALTWIN_DIGITALTWINTABWIDGET_H

#include <QWidget>


QT_BEGIN_NAMESPACE
namespace Ui { class DigitalTwinTabWidget; }
QT_END_NAMESPACE

class DigitalTwinTabWidget : public QWidget {
Q_OBJECT

public:
    DigitalTwinTabWidget(QWidget *parent = nullptr);

    ~DigitalTwinTabWidget() override;

private:
    void makeConnections();

    Ui::DigitalTwinTabWidget *ui;
};


#endif //DIGITALTWIN_DIGITALTWINTABWIDGET_H
