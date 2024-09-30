//
// Created by Moritz Herzog on 23.05.24.
//

#ifndef DIGITALTWIN_DIGITALTWINTABWIDGET_H
#define DIGITALTWIN_DIGITALTWINTABWIDGET_H

#include <QWidget>
#include <vector>

namespace DigitalTwin::Client {
    namespace Ui {
        class DigitalTwinTabWidget;
    }
    class VariableViewModel;
    class TabContentWidget;
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

        void generateModel();

        Ui::DigitalTwinTabWidget *ui;
        Model::DigitalTwinModel* Model;
        VariableViewModel* VariableModel;
        std::vector<TabContentWidget*> TabContents;

    private slots:
        void onVariableClicked(const QModelIndex & index);
    };
}

#endif //DIGITALTWIN_DIGITALTWINTABWIDGET_H
