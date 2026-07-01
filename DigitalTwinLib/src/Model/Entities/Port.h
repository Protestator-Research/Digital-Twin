//
// Created by Moritz Herzog on 25.09.24.
//

#ifndef DIGITALTWIN_PORT_H
#define DIGITALTWIN_PORT_H

#include "IDigitalTwinElement.h"

#include <string>
#include <vector>
#include "../../cpp_digital_twin_lib_global.h"

namespace DigitalTwin::Model {
    class Variable;

    class CPPDIGITALTWINLIB_EXPORT Port : public IDigitalTwinElement {
    public:
        Port() = delete;
        explicit Port(std::string name);

        ~Port() = default;

        void addVariable(Variable* variable);
        Variable* getVarialbe(std::string variableName);
    private:
        std::vector<Variable*> Variables;
    };
}

#endif //DIGITALTWIN_PORT_H
