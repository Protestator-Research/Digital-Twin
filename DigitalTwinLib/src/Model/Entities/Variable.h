//
// Created by Moritz Herzog on 11.09.24.
//

#ifndef DIGITALTWIN_VARIABLE_H
#define DIGITALTWIN_VARIABLE_H

#include <vector>

#include "IDigitalTwinElement.h"
#include "../SupportedTypes/SupportedTypes.h"
#include "../../cpp_digital_twin_lib_global.h"

namespace DigitalTwin::Model {
    template<typename T>
    class CPPDIGITALTWINLIB_EXPORT Variable : public IDigitalTwinElement {
    public:
        Variable() = delete;
        Variable(std::string name);
        Variable(std::string name, T value);
        ~Variable() override = default;

        void addLinkToVariable(Variable* variable);

        [[deprecated("Use getValue() instead")]]
        int getValueAsInt();
        [[deprecated("Use getValue() instead")]]
        char getValueAsChar();
        [[deprecated("Use getValue() instead")]]
        double getValueAsDouble();
        [[deprecated("Use getValue() instead")]]
        bool getValueAsBoolean();

        void setNewValue(T value);

        T getValue();

        Variable* copy();
         
    private:
        void updateLinkedVariables();
        void setVariableValueWithoutPropagation(T value);

        std::vector<Variable*> LinkedVariables;
        T Value;
    };
} // DigitalTwin::Model

#endif //DIGITALTWIN_VARIABLE_H
