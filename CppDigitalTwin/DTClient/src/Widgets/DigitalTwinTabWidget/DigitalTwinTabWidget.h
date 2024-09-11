//
// Created by Moritz Herzog on 23.05.24.
//

#ifndef DIGITALTWIN_DIGITALTWINTABWIDGET_H
#define DIGITALTWIN_DIGITALTWINTABWIDGET_H

#include <QWidget>

namespace DigitalTwin::Client {
    namespace Ui {
        class DigitalTwinTabWidget;
    }
}

namespace DigitalTwin::Model{
    class DigitalTwinModel;
}

namespace DigitalTwin::Client {

    class DigitalTwinTabWidget : public QWidget {
    Q_OBJECT

    public:
        DigitalTwinTabWidget(Model::DigitalTwinModel* model, QWidget *parent = nullptr);

        virtual ~DigitalTwinTabWidget() override;

    private:
        void makeConnections();

        Ui::DigitalTwinTabWidget *ui;
        Model::DigitalTwinModel* Model;
    };
}

#endif //DIGITALTWIN_DIGITALTWINTABWIDGET_H
