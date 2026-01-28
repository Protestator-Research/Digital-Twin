//
// Created by Moritz Herzog on 11.09.24.
//

#include "Variable.h"

#include <exception>

namespace DigitalTwin::Model {
    Variable::Variable(std::string name, SupportedTypes type) : IDigitalTwinElement(name) {
        Type = type;
        initValue();
    }

    void Variable::updateLinkedVariables() {
        for(auto variable : LinkedVariables)
            variable->setVariableValueWithoutPropagation(Value);
    }

    void Variable::setVariableValueWithoutPropagation(void* value) {
        if(Type == SupportedTypes::INT) {
            int* valueAsInt = (int*)Value;
            int* valueGetAsInt = (int*)value;
            *valueAsInt = *valueGetAsInt;
        }

        if(Type == SupportedTypes::CHAR) {
            char* valueAsChar= (char*)Value;
            char* valueGetAsChar = (char*)value;
            *valueAsChar = *valueGetAsChar;
        }

        if(Type == SupportedTypes::DOUBLE) {
            double* valueAsDouble = (double*)Value;
            double* valueGetAsDouble = (double*)value;
            *valueAsDouble = *valueGetAsDouble;
        }

        if(Type==SupportedTypes::BOOLEAN) {
            bool* valueAsDouble = (bool*)Value;
            bool* valueGetAsDouble = (bool*)value;
            *valueAsDouble = *valueGetAsDouble;
        }
    }

    void Variable::addLinkToVariable(Variable *variable) {
        LinkedVariables.push_back(variable);
    }

    int Variable::getValueAsInt() {
        if(Type==SupportedTypes::INT) {
            return *(int*)Value;
        }
        else
            throw std::exception();
    }

    char Variable::getValueAsChar() {
        if(Type==SupportedTypes::CHAR) {
            return *(char*)Value;
        }
        else
            throw std::exception();
    }

    double Variable::getValueAsDouble() {
        if(Type==SupportedTypes::DOUBLE) {
            return *(double*)Value;
        }
        else
            throw std::exception();
    }

    void Variable::setNewValue(int value) {
        if(Type != SupportedTypes::INT)
            throw new std::exception();

        *(int*)Value=value;
        updateLinkedVariables();
    }

    void Variable::setNewValue(double value) {
        if(Type != SupportedTypes::DOUBLE)
            throw new std::exception();

        *(double*)Value=value;
        updateLinkedVariables();
    }

    void Variable::setNewValue(char value) {
        if(Type != SupportedTypes::CHAR)
            throw new std::exception();

        *(char*)Value=value;
        updateLinkedVariables();
    }

    void Variable::initValue() {
        switch (Type) {
            case CHAR:
                Value = (void*)(new char);
                break;
            case INT:
                Value = (void*)(new int);
                break;
            case DOUBLE:
                Value = (void*)(new double);
                break;
            case BOOLEAN:
                Value = (void*)(new bool);
                break;
            default:
                throw new std::exception();
        }
    }

    bool Variable::getValueAsBoolean() {
        if(Type==SupportedTypes::BOOLEAN) {
            return *(bool*)Value;
        }
        else
            throw std::exception();
    }

    void Variable::setNewValue(bool value) {
        if(Type != SupportedTypes::BOOLEAN)
            throw new std::exception();

        *(bool *)Value=value;
        updateLinkedVariables();
    }
}