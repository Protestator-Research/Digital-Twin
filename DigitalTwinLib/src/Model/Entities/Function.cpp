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

        IVariable* Function::getReturnVariable() const
        {
            return ReturnValue;
        }
    } // Model
} // DigitalTwin