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
    class CPPDIGITALTWINLIB_EXPORT Variable : public IDigitalTwinElement {
    public:
        Variable() = delete;
        Variable(std::string name, SupportedTypes type);
        ~Variable() override = default;

        void addLinkToVariable(Variable* variable);

        int getValueAsInt();
        char getValueAsChar();
        double getValueAsDouble();
        bool getValueAsBoolean();

        void setNewValue(int value);
        void setNewValue(char value);
        void setNewValue(double value);
        void setNewValue(bool value);

    private:
        void updateLinkedVariables();
        void setVariableValueWithoutPropagation(void* value);
        void initValue();

        std::vector<Variable*> LinkedVariables;
        void* Value;
        SupportedTypes Type;
    };
} // DigitalTwin::Model

#endif //DIGITALTWIN_VARIABLE_H
