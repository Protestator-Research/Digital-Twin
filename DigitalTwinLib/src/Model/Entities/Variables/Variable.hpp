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
#include "../../State/StateTypes.h"
#include "../../../cpp_digital_twin_lib_global.h"

namespace DigitalTwin::Model {
    class IVariable;

    /**
     * Is told about every change of the value of a variable, including the changes that a linked variable
     * receives without propagation. A variable has at most one observer.
     */
    class CPPDIGITALTWINLIB_EXPORT IVariableObserver {
    public:
        virtual ~IVariableObserver() = default;

        /** Called after the value was set. The observer must not change the variable. */
        virtual void variableChanged(IVariable& variable) = 0;
    };

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

        /**
         * Sets the observer that is told about value changes, nullptr removes it. The observer is not owned and
         * must outlive the variable or be removed first. A copy of a variable has no observer.
         */
        void setObserver(IVariableObserver* observer)
        {
            Observer = observer;
        }

        IVariableObserver* getObserver() const
        {
            return Observer;
        }

        /** Value type in the history (Natural and Positive are Integer, scalar quantities are Real). */
        virtual ValueType getValueType() const = 0;

        /** The current value in the column encoding of the history. */
        virtual VariableValue getStateValue() const = 0;

        /**
         * Sets the value from a stored state without propagating it to linked variables.
         * @return false (and the value is unchanged) if the value has another type or is malformed.
         */
        virtual bool setStateValue(const VariableValue& value) = 0;

    protected:
        virtual void updateLinkedVariables() = 0;

        /** Reports the new value to the observer. To be called by every function that sets the value. */
        void notifyChanged()
        {
            if (Observer)
                Observer->variableChanged(*this);
        }

        std::vector<IVariable*> LinkedVariables;
        Function* FunctionInvocation;

    private:
        IVariableObserver* Observer = nullptr;
    };
} // DigitalTwin::Model

#endif //DIGITALTWIN_VARIABLE_H
