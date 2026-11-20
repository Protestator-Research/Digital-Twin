//
// Created by herzog on 01.09.26.
//

#pragma once

#include <complex>

#include "Variable.hpp"

namespace DigitalTwin::Model
{
    class CPPDIGITALTWINLIB_EXPORT ComplexVariable : public IVariable
    {
    public:
        ComplexVariable() = delete;
        ComplexVariable(std::string name);
        ComplexVariable(std::string name, const double& real, const double& imaginary);
        ComplexVariable(std::string name, const std::complex<double>& value);
        virtual ~ComplexVariable()  override = default;

        std::complex<double> getValue() const;
        void setValue(const double& real, const double& imaginary);
        void setValue(const std::complex<double>& value);
        IVariable* copy() override;
        std::string getType() override;

        ValueType getValueType() const override;
        VariableValue getStateValue() const override;
        bool setStateValue(const VariableValue& value) override;

	protected:
        void updateLinkedVariables() override;

        void setValueWithoutPropagation(std::complex<double> value);
    private:
        std::complex<double> Value;

    };
}

