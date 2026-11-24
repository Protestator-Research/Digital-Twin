//
// Created by herzog on 04.08.26.
//

#include "Variables/Variable.hpp"
#include "Function.h"

namespace DigitalTwin
{
    namespace Model
    {
        Function::Function(const std::string& name) :
        IDigitalTwinElement(name),
        ReturnValue(nullptr)
        {
        }

        Function::~Function()
        {
        }

        std::vector<IVariable*> Function::getParameters() const
        {
            return Parameters;
        }

        void Function::appendParameter(IVariable* variable)
        {
            Parameters.push_back(variable);
        }

        const std::vector<std::string>& Function::getParameterNames() const
        {
            return ParameterNames;
        }

        void Function::appendParameterName(const std::string& name)
        {
            ParameterNames.push_back(name);
        }

        IVariable* Function::getReturnVariable() const
        {
            return ReturnValue;
        }

        void Function::setReturnVariable(IVariable* variable)
        {
            ReturnValue = variable;
        }

        Function* Function::invocate()
        {
            const auto function = new Function(Name);
            function->ParameterNames = ParameterNames;
            function->ReturnValue = ReturnValue;
            return function;
        }

        Function* Function::rebind(const std::map<IVariable*, IVariable*>& replacements, IVariable* returnVariable) const
        {
            const auto function = new Function(Name);
            function->ParameterNames = ParameterNames;
            for (const auto argument : Parameters)
            {
                const auto replacement = replacements.find(argument);
                function->Parameters.push_back(replacement != replacements.end() ? replacement->second : argument);
            }
            function->ReturnValue = returnVariable;
            return function;
        }
    } // Model
} // DigitalTwin