//
// Created by Moritz Herzog on 25.09.24.
//

#ifndef DIGITALTWIN_PORT_H
#define DIGITALTWIN_PORT_H


#include "IDigitalTwinElement.h"

#include <string>
#include <map>
#include <vector>

#include "ICollectionType.h"
#include "../../cpp_digital_twin_lib_global.h"

namespace DigitalTwin::Model {
    class Variable;

    class CPPDIGITALTWINLIB_EXPORT Port : public ICollectionType {
    public:
        Port() = delete;
        explicit Port(std::string name);

        ~Port() = default;

        void appendAttribute(Variable* variable) override;
        Variable* getAttribute(std::string variableName);
        void appendComponent(Component* compoonent) override;
        void appendPort(Port* port) override;
        void appendControllable(Variable* variable) override;
        void appendMeasurable(Variable* variable) override;
        Variable* resolveVariable(std::string name) override;
        Variable* resolveVariable(std::vector<std::string> domains, size_t index) override;

    private:

    };
}

#endif //DIGITALTWIN_PORT_H
