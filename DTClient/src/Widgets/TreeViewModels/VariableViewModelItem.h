//
// Created by Moritz Herzog on 26.09.24.
//

#ifndef DIGITALTWIN_VARIABLEVIEWMODELITEM_H
#define DIGITALTWIN_VARIABLEVIEWMODELITEM_H

#include <vector>
#include <Model/Entities/Component.h>
#include <Model/Entities/Variables/Variable.hpp>
#include <Model/Entities/Function.h>
#include <QVariant>
#include <any>

namespace DigitalTwin::Client {
    class VariableViewModelItem {
    public:
        VariableViewModelItem() = default;
        explicit VariableViewModelItem(DigitalTwin::Model::Component* component, VariableViewModelItem* parent = nullptr);
        explicit VariableViewModelItem(DigitalTwin::Model::IVariable* variable, VariableViewModelItem* parent = nullptr);
        explicit VariableViewModelItem(DigitalTwin::Model::Function* function, VariableViewModelItem* parent = nullptr);
        explicit VariableViewModelItem(std::string displayString, VariableViewModelItem* parent = nullptr);

        ~VariableViewModelItem() = default;

        VariableViewModelItem* child(int row);
        int childCount();
        int columnCount();
        QVariant data();
        int row();
        VariableViewModelItem* parent();

        void appendVariable(DigitalTwin::Model::IVariable* variable);
        DigitalTwin::Model::IVariable* getVariable() const;
        void appendComponent(DigitalTwin::Model::Component* component);
        DigitalTwin::Model::Component* getComponent() const;
        void appendFunction(DigitalTwin::Model::Function* function);
        DigitalTwin::Model::Function* getFunction() const;

    private:
        void generateComponentView();

        VariableViewModelItem* Parent;

        std::vector<VariableViewModelItem*> ChildItems;

        DigitalTwin::Model::Component* Component = nullptr;
        DigitalTwin::Model::IVariable* Variable = nullptr;
        DigitalTwin::Model::Function* Function = nullptr;

        std::string Text = "";

        enum VariableViewModelItemType {
            DisplayType,
            ComponentType,
            VariableType,
            FunctionType
        };

        VariableViewModelItemType Type;
    };
}

#endif //DIGITALTWIN_VARIABLEVIEWMODELITEM_H
