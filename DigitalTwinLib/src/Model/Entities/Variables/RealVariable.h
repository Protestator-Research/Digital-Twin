//
// Created by herzog on 01.09.26.
//

#pragma once

#include "Variable.hpp"

namespace DigitalTwin::Model
{
    class CPPDIGITALTWINLIB_EXPORT RealVariable : public IVariable
    {
    public:
        RealVariable() = delete;

        explicit RealVariable(std::string name);
        explicit RealVariable(std::string name, double value);

        virtual ~RealVariable() = default;

        IVariable* copy() override;
        std::string getType() override;

        ValueType getValueType() const override;
        VariableValue getStateValue() const override;
        bool setStateValue(const VariableValue& value) override;

        double getValue() const;
        void setValue(const double& value);

	protected:
        void updateLinkedVariables() override;

        void setValueWithoutPropagation(const double& value);

    private:
        double Value;
    };
}
