//
// Created by Moritz Herzog on 08.08.24.
//

#include "Component.h"
#include "Variable.h"

namespace DigitalTwin::Model {
    Component::Component(std::string name) : IDigitalTwinElement(name) {

    }

    Component::~Component() {
        for(const auto & elem : ComponentMap)
            delete elem.second;

        ComponentMap.clear();

        for(const auto & elem : Controllables)
            delete elem.second;

        Controllables.clear();

        for(const auto & elem : Measurables)
            delete elem.second;

        Measurables.clear();

    }

    void Component::appendComponent(Component *component) {
        ComponentMap.insert(std::make_pair(component->Name, component));
    }

    void Component::appendMeasurable(Variable *variable) {
        Measurables.insert(std::make_pair(variable->getName(),variable));
    }

    void Component::appendControllable(Variable *variable) {
        Measurables.insert(std::make_pair(variable->getName(),variable));
    }

    void Component::appendAttribute(Variable *variable) {

        Attributes.insert(std::make_pair(variable->getName(),variable));
    }

    Variable *Component::getControllable(std::string name) {
        return Controllables.at(name);
    }

    Variable *Component::getMeasurable(std::string name) {
        return Measurables.at(name);
    }

    Component *Component::getComponent(std::string name) {
        return ComponentMap.at(name);
    }

    Variable *Component::getAttribute(std::string name) {
        return Attributes.at(name);
    }

    std::vector<Component *> Component::getAllComponents() {
        std::vector<Component*> components;

        for(auto element : ComponentMap)
            components.push_back(element.second);

        return components;
    }

    std::vector<Variable *> Component::getAllVariables() {
        std::vector<Variable*> variables;

        for(auto element : Controllables)
            variables.push_back(element.second);

        for(auto element : Measurables)
            variables.push_back(element.second);

        return variables;
    }
}