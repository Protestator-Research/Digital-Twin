//
// Created by Moritz Herzog on 11.09.24.
//

#ifndef DIGITALTWIN_VARIABLE_H
#define DIGITALTWIN_VARIABLE_H

#include <vector>
#include <boost/hana/value.hpp>

#include "../Function.h"
#include "../IDigitalTwinElement.h"
#include "../../SupportedTypes/SupportedTypes.h"
#include "../../../cpp_digital_twin_lib_global.h"

namespace DigitalTwin::Model {
    class CPPDIGITALTWINLIB_EXPORT IVariable : public IDigitalTwinElement {
    public:
        IVariable() = delete;
        IVariable(std::string name) : IDigitalTwinElement(name)
        {
            FunctionInvocation = nullptr;
        }

        ~IVariable() override = default;

        void addLinkToVariable(IVariable* variable)
        {
            LinkedVariables.push_back(variable);
        }

        virtual IVariable* copy() = 0;

        virtual std::string getType() = 0;

        void setFunctionInvocation(Function* function)
        {
            FunctionInvocation = function;
        }

        Function* getFunctionInvovation() const
        {
            return FunctionInvocation;
        }

    protected:
        virtual void updateLinkedVariables() = 0;

        std::vector<IVariable*> LinkedVariables;
        Function* FunctionInvocation;
    };
} // DigitalTwin::Model

#endif //DIGITALTWIN_VARIABLE_H
