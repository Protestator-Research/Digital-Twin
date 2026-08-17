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
    class CPPDIGITALTWINLIB_EXPORT Port : public ICollectionType {
    public:
        Port() = delete;
        explicit Port(std::string name);

        ~Port() = default;

        void appendAttribute(Variable<std::any>* variable) override;
        Variable<std::any>* getAttribute(std::string variableName);
        void appendComponent(Component* compoonent) override;
        void appendPort(Port* port) override;
        void appendControllable(Variable<std::any>* variable) override;
        void appendMeasurable(Variable<std::any>* variable) override;
        Variable<std::any>* resolveVariable(std::string name) override;
        Variable<std::any>* resolveVariable(std::vector<std::string> domains, size_t index) override;

    private:

    };
}

#endif //DIGITALTWIN_PORT_H
