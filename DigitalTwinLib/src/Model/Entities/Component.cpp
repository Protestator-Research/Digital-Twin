//
// Created by Moritz Herzog on 08.08.24.
//

#include <BaseFuctions/StringExtention.hpp>

#include "../Exceptions/DigitalTwinAddressException.h"
#include "Component.h"

#include "Port.h"
#include "Variables/Variable.hpp"

namespace DigitalTwin::Model {
    Component::Component(std::string name) : ICollectionType(name) {

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

    void Component::appendPort(Port* port)
    {
        PortMap.insert(std::make_pair(port->getName(), port));
    }

    void Component::appendComponent(Component *component) {
        ComponentMap.insert(std::make_pair(component->Name, component));
    }

    void Component::appendMeasurable(Variable<std::any>* variable) {
        Measurables.insert(std::make_pair(variable->getName(),variable));
    }

    void Component::appendControllable(Variable<std::any>* variable) {
        Controllables.insert(std::make_pair(variable->getName(),variable));
    }

    void Component::appendAttribute(Variable<std::any>* variable) {

        Attributes.insert(std::make_pair(variable->getName(),variable));
    }

    Variable<std::any>* Component::getControllable(std::string name) {
        return Controllables.at(name);
    }

    Variable<std::any>* Component::getMeasurable(std::string name) {
        return Measurables.at(name);
    }

    Component *Component::getComponent(std::string name) {
        return ComponentMap.at(name);
    }

    Port* Component::getPort(std::string name)
    {
        return PortMap.at(name);
    }

    Variable<std::any>* Component::getAttribute(std::string name) {
        return Attributes.at(name);
    }

    std::vector<Component *> Component::getAllComponents() {
        std::vector<Component*> components;

        for(auto element : ComponentMap)
            components.push_back(element.second);

        return components;
    }

    std::vector<Variable<std::any>*> Component::getAllVariables() {
        std::vector<Variable<std::any>*> variables;

        for (const auto& elem : Attributes)
            variables.push_back(elem.second);

        for(const auto& element : Controllables)
            variables.push_back(element.second);

        for(const auto& element : Measurables)
            variables.push_back(element.second);

        return variables;
    }

    std::vector<Port*> Component::getAllPorts()
    {
        std::vector<Port*> ports;

        for (auto element : PortMap)
            ports.push_back(element.second);

        return ports;
    }

    std::vector<std::string> Component::getAllMQTTTopics() {
        std::vector<std::string> returnValue;

        for(const auto& comp : ComponentMap)
            for(const auto& string : dynamic_cast<Component*>(comp.second)->getAllMQTTTopics())
                returnValue.push_back(comp.first + "/" + string);

        for(auto element : Controllables)
            returnValue.push_back(element.first);

        for(auto element : Measurables)
            returnValue.push_back(element.first);

        return returnValue;
    }

    Component* Component::instantiate(std::string name)
    {
        auto comp = new Component(name);
        
        for (const auto& [comp_name, component] : ComponentMap)
            comp->appendComponent(component->instantiate(comp_name));

        for (auto [_, controllable] : Controllables)
            comp->appendControllable(controllable->copy());

        for (auto [_, measurable] : Measurables)
            comp->appendMeasurable(measurable);

        for (auto [_, attribute] : Attributes)
            comp->appendMeasurable(attribute);

        return comp;
    }

    Variable<std::any>* Component::resolveVariable(std::string name) {
        auto splittedAdress = CPSBASELIB::STD_EXTENTION::StringExtention::splitString(name, '/');

        if (splittedAdress.size() == 1)
            splittedAdress = CPSBASELIB::STD_EXTENTION::StringExtention::splitString(name, '.');

        return resolveVariable(splittedAdress, 0);
    }

    Variable<std::any>* Component::resolveVariable(std::vector<std::string> domains, size_t index)
    {
        if (index >= domains.size())
            throw DigitalTwinAddressException();

        if ((size_t)index == (domains.size()-1))
        {
            if (Controllables.contains(domains.back()))
                return Controllables.at(domains.back());
            if (Measurables.contains(domains.back()))
                return Measurables.at(domains.back());
            if (Attributes.contains(domains.back()))
                return Attributes.at(domains.back());
            throw DigitalTwinAddressException();
        }

        return ComponentMap[domains[index]]->resolveVariable(domains, index + 1);
    } 
}
