//
// Created by Moritz Herzog on 11.09.24.
//

#ifndef DIGITALTWIN_VARIABLE_H
#define DIGITALTWIN_VARIABLE_H

#include <vector>
#include <boost/hana/value.hpp>

#include "IDigitalTwinElement.h"
#include "../SupportedTypes/SupportedTypes.h"
#include "../../cpp_digital_twin_lib_global.h"

namespace DigitalTwin::Model {
    template<typename T>
    class CPPDIGITALTWINLIB_EXPORT Variable : public IDigitalTwinElement {
    public:
        Variable() = delete;
        Variable(std::string name) : IDigitalTwinElement(name) {}
        Variable(std::string name, T value) : IDigitalTwinElement(name)
        {
            Value = value;
        }

        ~Variable() override = default;

        void addLinkToVariable(Variable<T>* variable)
        {
            LinkedVariables.push_back(variable);
        }

        [[deprecated("Use getValue() instead")]]
        int getValueAsInt()
        {
            if(std::same_as<T,int>) {
                return Value;
            }
            else
                throw std::exception();
        }
        [[deprecated("Use getValue() instead")]]
        char getValueAsChar()
        {
            if(std::same_as<T,char>) {
                return Value;
            }
            else
                throw std::exception();
        }
        [[deprecated("Use getValue() instead")]]
        double getValueAsDouble()
        {
            if(std::same_as<T,double>) {
                return Value;
            }
            else
                throw std::exception();
        }
        [[deprecated("Use getValue() instead")]]
        bool getValueAsBoolean()
        {
            if(std::same_as<T,bool>) {
                return Value;
            }
            else
                throw std::exception();
        }

        void setNewValue(T value)
        {
            Value = value;
            updateLinkedVariables();
        }

        T getValue()
        {
            return Value;
        }

        Variable<T>* copy()
        {
            return new Variable<T>(Name, Value);
        }
         
    private:
        void updateLinkedVariables();
        void setVariableValueWithoutPropagation(T value);

        std::vector<Variable<T>*> LinkedVariables;
        T Value;
    };
} // DigitalTwin::Model

#endif //DIGITALTWIN_VARIABLE_H
