//
// Created by herzog on 04.08.26.
//

#pragma once

#include <vector>
#include <string>
#include <any>
#include <map>

#include "IDigitalTwinElement.h"
#include "../../cpp_digital_twin_lib_global.h"

namespace DigitalTwin::Model
{
    class IVariable;
    class CPPDIGITALTWINLIB_EXPORT Function : public IDigitalTwinElement
    {
    public:
        Function() = delete;

        explicit Function(const std::string& name);

        ~Function() override;

        std::vector<IVariable*> getParameters() const;
        void appendParameter(IVariable* variable);
        /**
         * Formal parameter names from the function definition, in declaration order.
         */
        const std::vector<std::string>& getParameterNames() const;
        /**
         * Appends the name of the next formal parameter of the function definition.
         */
        void appendParameterName(const std::string& name);
        IVariable* getReturnVariable() const;
        void setReturnVariable(IVariable* variable);
        /**
         * Creates an invocation: copies the formal parameter names and the return variable. The Parameters of the
         * invocation are empty and must be filled with the actual arguments, in order.
         */
        Function* invocate();
        /**
         * Creates a copy of this invocation for a copied variable. Name and formal parameter names are taken over.
         * Every actual argument is replaced by its entry in replacements if present, otherwise it is kept as is
         * (nullptr stays nullptr). The return variable of the copy is returnVariable.
         */
        Function* rebind(const std::map<IVariable*, IVariable*>& replacements, IVariable* returnVariable) const;
    private:
        std::vector<IVariable*> Parameters;
        std::vector<std::string> ParameterNames;
        IVariable* ReturnValue;
    };
} // DigitalTwin::Model

