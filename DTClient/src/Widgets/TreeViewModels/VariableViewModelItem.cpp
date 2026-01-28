//
// Created by Moritz Herzog on 26.09.24.
//

#include "VariableViewModelItem.h"
namespace DigitalTwin::Client {

    VariableViewModelItem::VariableViewModelItem(DigitalTwin::Model::Component *component,
                                                 VariableViewModelItem *parent) {
        Parent = parent;
        Component = component;
        Type = ComponentType;
        generateComponentView();
    }

    VariableViewModelItem::VariableViewModelItem(DigitalTwin::Model::Variable *variable,
                                                 VariableViewModelItem *parent) {
        Parent = parent;
        Variable = variable;
        Type = VariableType;
    }

    VariableViewModelItem::VariableViewModelItem(std::string displayString, VariableViewModelItem *parent) {
        Text = displayString;
        Parent = parent;
        Type = DisplayType;
    }

    VariableViewModelItem *VariableViewModelItem::child(int row) {
        return row >= 0 && row < childCount() ? ChildItems.at(row) : nullptr;;
    }

    int VariableViewModelItem::childCount() {
        return int(ChildItems.size());
    }

    int VariableViewModelItem::columnCount() {
        return 1;
    }

    QVariant VariableViewModelItem::data() {
        switch (Type) {
            case DisplayType:
                return QVariant(QString::fromStdString(Text));
            case ComponentType:
                if(Component!= nullptr)
                    return QVariant(QString::fromStdString(Component->getName()));
                else
                    return QVariant("Error: Component was null");
            case VariableType:
                return QVariant(QString::fromStdString(Variable->getName()));
        }
        return QVariant();
    }

    int VariableViewModelItem::row() {
        if(Parent == nullptr)
            return 0;
        const auto it = std::find_if(Parent->ChildItems.cbegin(),Parent->ChildItems.cend(),
                                     [this](VariableViewModelItem *treeItem){
                                         return treeItem==this;
                                    });
        if(it!=Parent->ChildItems.cend())
            return std::distance(Parent->ChildItems.cbegin(),it);

        Q_ASSERT(false);
        return -1;
    }

    VariableViewModelItem *VariableViewModelItem::parent() {
        return Parent;
    }

    void VariableViewModelItem::appendVariable(DigitalTwin::Model::Variable *variable) {
        ChildItems.push_back(new VariableViewModelItem(variable, this));
    }

    DigitalTwin::Model::Variable *VariableViewModelItem::getVariable() const {
        return Variable;
    }

    void VariableViewModelItem::appendComponent(DigitalTwin::Model::Component *component) {
        ChildItems.push_back(new VariableViewModelItem(component, this));
    }

    DigitalTwin::Model::Component *VariableViewModelItem::getComponent() const {
        return Component;
    }

    void VariableViewModelItem::generateComponentView() {
        if(Component->getAllVariables().size()>0) {
            auto variableElement = new VariableViewModelItem("Variables", this);
            ChildItems.push_back(variableElement);
            for (auto element: Component->getAllVariables())
                variableElement->appendVariable(element);
        }

        if(Component->getAllComponents().size()>0) {
            auto componentElement = new VariableViewModelItem("Components", this);
            ChildItems.push_back(componentElement);
            for (auto element: Component->getAllComponents())
                componentElement->appendComponent(element);
        }
    }

}