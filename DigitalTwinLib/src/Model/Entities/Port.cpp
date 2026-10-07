//
// Created by Moritz Herzog on 25.09.24.
//

#include "Port.h"
#include "../Exceptions/DigitalTwinAddressException.h"
#include "Component.h"
#include "Variables/Variable.hpp"
#include "BaseFuctions/StringExtention.hpp"

namespace DigitalTwin::Model {
    Port::Port(std::string name) : ICollectionType(name) {

    }

    void Port::appendAttribute(IVariable* variable)
    {
        Attributes.insert(std::make_pair(variable->getName(),variable));
    }

    IVariable* Port::getAttribute(std::string variableName)
    {
        return Attributes.at(variableName);
    }

    void Port::appendComponent(Component* compoonent)
    {
        ComponentMap.insert(std::make_pair(compoonent->getName(), compoonent));
    }

    void Port::appendPort(Port* port)
    {
        PortMap.insert(std::make_pair(port->getName(), port));
    }

    void Port::appendControllable(IVariable* variable)
    {
        Controllables.insert(std::make_pair(variable->getName(), variable));
    }

    void Port::appendMeasurable(IVariable* variable)
    {
        Measurables.insert(std::make_pair(variable->getName(), variable));
    }

    IVariable* Port::resolveVariable(std::string name)
    {
        auto splittedAdress = CPSBASELIB::STD_EXTENTION::StringExtention::splitString(name, '/');

        if (splittedAdress.size() == 1)
            splittedAdress = CPSBASELIB::STD_EXTENTION::StringExtention::splitString(name, '.');

        return resolveVariable(splittedAdress, 0);
    }

    IVariable* Port::resolveVariable(std::vector<std::string> domains, size_t index)
    {
        if (index >= domains.size())
            throw DigitalTwinAddressException();

        if ((size_t)index == (domains.size() - 1))
        {
            if (Controllables.contains(domains.back()))
                return Controllables.at(domains.back());
            if (Measurables.contains(domains.back()))
                return Measurables.at(domains.back());
            if (Attributes.contains(domains.back()))
                return Attributes.at(domains.back());
            throw DigitalTwinAddressException();
        }

        return dynamic_cast<Component*>(ComponentMap[domains[index]])->resolveVariable(domains, index + 1);
    }
}
