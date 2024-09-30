//
// Created by Moritz Herzog on 26.09.24.
//

#ifndef DIGITALTWIN_VARIABLEVIEWMODELITEM_H
#define DIGITALTWIN_VARIABLEVIEWMODELITEM_H

#include <vector>
#include <Model/Entities/Component.h>
#include <Model/Entities/Variable.h>
#include <QVariant>

namespace DigitalTwin::Client {
    class VariableViewModelItem {
    public:
        VariableViewModelItem() = default;
        explicit VariableViewModelItem(DigitalTwin::Model::Component* component, VariableViewModelItem* parent = nullptr);
        explicit VariableViewModelItem(DigitalTwin::Model::Variable* variable, VariableViewModelItem* parent = nullptr);
        explicit VariableViewModelItem(std::string displayString, VariableViewModelItem* parent = nullptr);

        ~VariableViewModelItem() = default;

        VariableViewModelItem* child(int row);
        int childCount();
        int columnCount();
        QVariant data();
        int row();
        VariableViewModelItem* parent();

        void appendVariable(DigitalTwin::Model::Variable* variable);
        DigitalTwin::Model::Variable* getVariable() const;
        void appendComponent(DigitalTwin::Model::Component* component);
        DigitalTwin::Model::Component* getComponent() const;

    private:
        void generateComponentView();

        VariableViewModelItem* Parent;

        std::vector<VariableViewModelItem*> ChildItems;

        DigitalTwin::Model::Component* Component = nullptr;
        DigitalTwin::Model::Variable* Variable = nullptr;
        std::string Text = "";

        enum VariableViewModelItemType {
            DisplayType,
            ComponentType,
            VariableType
        };

        VariableViewModelItemType Type;
    };
}

#endif //DIGITALTWIN_VARIABLEVIEWMODELITEM_H
