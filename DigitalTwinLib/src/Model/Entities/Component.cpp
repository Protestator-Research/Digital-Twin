//
// Created by Moritz Herzog on 08.08.24.
//

#include <BaseFuctions/StringExtention.hpp>

#include "../Exceptions/DigitalTwinAddressException.h"
#include "Component.h"
#include <iostream>

#include "Port.h"
#include "Function.h"
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

    void Component::appendMeasurable(IVariable* variable) {
        Measurables.insert(std::make_pair(variable->getName(),variable));
    }

    void Component::appendControllable(IVariable* variable) {
        Controllables.insert(std::make_pair(variable->getName(),variable));
    }

    void Component::appendAttribute(IVariable* variable) {

        Attributes.insert(std::make_pair(variable->getName(),variable));
    }

    IVariable* Component::getControllable(std::string name) {
        return Controllables.at(name);
    }

    IVariable* Component::getMeasurable(std::string name) {
        return Measurables.at(name);
    }

    Component *Component::getComponent(std::string name) {
        if (ComponentMap.contains(name))
            return ComponentMap.at(name);
        return nullptr;
    }

    Port* Component::getPort(std::string name)
    {
        return PortMap.at(name);
    }

    IVariable* Component::getAttribute(std::string name) {
        return Attributes.at(name);
    }

    std::vector<Component *> Component::getAllComponents() {
        std::vector<Component*> components;

        for(auto element : ComponentMap)
            components.push_back(element.second);

        return components;
    }

    std::vector<IVariable*> Component::getAllVariables() {
        std::vector<IVariable*> variables;

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

    std::vector<Function*> Component::getAllFunctions()
    {
        std::vector<Function*> functions;

        for (auto element : Functions)
            functions.push_back(element.second);

        return functions;
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

        for (auto element : Functions)
            returnValue.push_back(element.first);

        return returnValue;
    }

    std::vector<std::pair<std::string, IVariable*>> Component::getAllMQTTVariables() {
        std::vector<std::pair<std::string, IVariable*>> returnValue;

        for(const auto& comp : ComponentMap)
            for(const auto& variable : dynamic_cast<Component*>(comp.second)->getAllMQTTVariables())
                returnValue.push_back(std::make_pair(comp.first + "/" + variable.first, variable.second));

        for(auto element : Controllables)
            returnValue.push_back(std::make_pair(element.first, element.second));

        for(auto element : Measurables)
            returnValue.push_back(std::make_pair(element.first, element.second));

        // TODO: Functions are excluded on purpose. They only get an MQTT interface after initialization,
        //  and that interface still has to be designed.

        return returnValue;
    }

    Component* Component::instantiate(std::string name)
    {
        auto comp = new Component(name);
        
        for (const auto& [comp_name, component] : ComponentMap)
            comp->appendComponent(component->instantiate(comp_name));

        // Controllables are copied; a copy needs its own function invocation, bound to the copies.
        std::map<IVariable*, IVariable*> copies;
        for (auto [_, controllable] : Controllables)
        {
            const auto copy = controllable->copy();
            copies[controllable] = copy;
            comp->appendControllable(copy);
        }
        for (const auto& [original, copy] : copies)
            if (const auto invocation = original->getFunctionInvovation())
                copy->setFunctionInvocation(invocation->rebind(copies, copy));

        for (auto [_, measurable] : Measurables)
            comp->appendMeasurable(measurable);

        for (auto [_, attribute] : Attributes)
            comp->appendAttribute(attribute);

        return comp;
    }

    IVariable* Component::resolveVariable(std::string name) {
        auto splittedAdress = CPSBASELIB::STD_EXTENTION::StringExtention::splitString(name, '/');

        if (splittedAdress.size() == 1)
            splittedAdress = CPSBASELIB::STD_EXTENTION::StringExtention::splitString(name, '.');

        return resolveVariable(splittedAdress, 0);
    }

    IVariable* Component::resolveVariable(std::vector<std::string> domains, size_t index)
    {
        if (index >= domains.size())
            throw DigitalTwinAddressException();

        //std::cout << "Component::resolveVariable" << std::endl;

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
        
        //std::cout << "Variable is in subpart." << std::endl;

    	if (ComponentMap.contains(domains[index]))
			return ComponentMap[domains[index]]->resolveVariable(domains, index + 1);
        if (PortMap.contains(domains[index]))
            return PortMap[domains[index]]->resolveVariable(domains, index + 1);
        throw DigitalTwinAddressException();
    } 
}
