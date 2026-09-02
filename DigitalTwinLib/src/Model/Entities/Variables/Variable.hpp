//
// Created by Moritz Herzog on 11.09.24.
//

#ifndef DIGITALTWIN_VARIABLE_H
#define DIGITALTWIN_VARIABLE_H

#include <vector>
#include <boost/hana/value.hpp>

#include "../IDigitalTwinElement.h"
#include "../../SupportedTypes/SupportedTypes.h"
#include "../../../cpp_digital_twin_lib_global.h"

namespace DigitalTwin::Model {
    class CPPDIGITALTWINLIB_EXPORT IVariable : public IDigitalTwinElement {
    public:
        IVariable() = delete;
        IVariable(std::string name) : IDigitalTwinElement(name) {}

        ~IVariable() override = default;

        void addLinkToVariable(IVariable* variable)
        {
            LinkedVariables.push_back(variable);
        }

    protected:
        virtual void updateLinkedVariables() = 0;

        std::vector<IVariable*> LinkedVariables;
    };
} // DigitalTwin::Model

#endif //DIGITALTWIN_VARIABLE_H
