//
// Created by Moritz Herzog on 25.09.24.
//

#include "Port.h"
#include "Variable.h"

namespace DigitalTwin::Model {
    Port::Port(std::string name) : IDigitalTwinElement(name) {

    }

    void Port::addVariable(Variable* variable)
    {
        Variables.push_back(variable);
    }

    Variable* Port::getVarialbe(std::string variableName)
    {
        for (const auto variable : Variables)
            if (variable->getName() == variableName)
                return variable;
        return nullptr;
    }
}
