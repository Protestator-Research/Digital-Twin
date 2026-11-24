//
// Created by herzog on 01.09.26.
//

#include "PositiveVariable.h"

#include <iostream>
#include <limits>

#include "../../State/ValueCodec.h"

namespace DigitalTwin::Model
{
	PositiveVariable::PositiveVariable(std::string name) : 
	IVariable(name)
	{
		Value = 0;   // was left uninitialised, the state of the model must always be readable
	}

	PositiveVariable::PositiveVariable(std::string name, unsigned int value) : 
	IVariable(name)
	{
		Value = value;
	}

	IVariable* PositiveVariable::copy()
	{
		return new PositiveVariable(Name, Value);
	}

	unsigned int PositiveVariable::getValue() const
	{
		return Value;
	}

	void PositiveVariable::setValue(const unsigned int& value)
	{
		Value = value;
		notifyChanged();
		updateLinkedVariables();
	}

	ValueType PositiveVariable::getValueType() const
	{
		return ValueType::Integer;
	}

	VariableValue PositiveVariable::getStateValue() const
	{
		return ValueCodec::encodeInteger(static_cast<int64_t>(Value));
	}

	bool PositiveVariable::setStateValue(const VariableValue& value)
	{
		int64_t decoded = 0;
		if (!ValueCodec::decodeInteger(value, decoded))
			return false;
		if (decoded < 0 || decoded > static_cast<int64_t>(std::numeric_limits<unsigned int>::max()))
			return false;
		setValueWithoutPropagation(static_cast<unsigned int>(decoded));
		return true;
	}

	std::string PositiveVariable::getType()
	{
		return "PositiveVariable";
	}

	void PositiveVariable::updateLinkedVariables()
	{
		for (const auto& variable : LinkedVariables)
		{
			try
			{
				dynamic_cast<PositiveVariable*>(variable)->setValueWithoutPropagation(Value);
			}
			catch (...)
			{
				std::cerr << "Issue wih casting Variable with name: " << variable->getName() << std::endl;
			}
		}
	}

	void PositiveVariable::setValueWithoutPropagation(const unsigned int& value)
	{
		Value = value;
		notifyChanged();
	}
}

