//
// Created by herzog on 01.09.26.
//

#include "ComplexVariable.h"

#include <iostream>

#include "../../State/ValueCodec.h"

namespace DigitalTwin::Model
{

	ComplexVariable::ComplexVariable(std::string name) : IVariable(name)
	{
		Value = std::complex<double>();
	}

	ComplexVariable::ComplexVariable(std::string name, const double& real, const double& imaginary) : IVariable(name)
	{
		Value = std::complex<double>(real, imaginary);
	}

	ComplexVariable::ComplexVariable(std::string name, const std::complex<double>& value) : IVariable(name)
	{
		Value = value;
	}

	std::complex<double> ComplexVariable::getValue() const
	{
		return Value;
	}

	void ComplexVariable::setValue(const double& real, const double& imaginary)
	{
		Value = std::complex<double>(real, imaginary);
		notifyChanged();
	}

	void ComplexVariable::setValue(const std::complex<double>& value)
	{
		Value = value;
		notifyChanged();
		updateLinkedVariables();
	}

	ValueType ComplexVariable::getValueType() const
	{
		return ValueType::Complex;
	}

	VariableValue ComplexVariable::getStateValue() const
	{
		return ValueCodec::encodeComplex(Value.real(), Value.imag());
	}

	bool ComplexVariable::setStateValue(const VariableValue& value)
	{
		double real = 0.0;
		double imaginary = 0.0;
		if (!ValueCodec::decodeComplex(value, real, imaginary))
			return false;
		setValueWithoutPropagation(std::complex<double>(real, imaginary));
		return true;
	}

	IVariable* ComplexVariable::copy()
	{
		return new ComplexVariable(Name, Value);
	}

	std::string ComplexVariable::getType()
	{
		return "ComplexVariable";
	}

	void ComplexVariable::updateLinkedVariables()
	{
		for (const auto& variable : LinkedVariables)
		{
			try
			{
				dynamic_cast<ComplexVariable*>(variable)->setValueWithoutPropagation(Value);
			}
			catch (...)
			{
				std::cerr << "Issue wih casting Variable with name: " << variable->getName() << std::endl;
			}
		}
	}

	void ComplexVariable::setValueWithoutPropagation(std::complex<double> value)
	{
		Value = value;
		notifyChanged();
	}

}