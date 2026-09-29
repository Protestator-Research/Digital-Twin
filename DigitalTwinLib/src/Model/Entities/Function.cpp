//
// Created by herzog on 04.08.26.
//

#include "Function.h"

namespace DigitalTwin
{
    namespace Model
    {
        Function::Function(const std::string& name) :
        IDigitalTwinElement(name)
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

        IVariable* Function::getReturnVariable() const
        {
            return ReturnValue;
        }

        void Function::setReturnVariable(IVariable* variable)
        {
            ReturnValue = variable;
        }
    } // Model
} // DigitalTwin