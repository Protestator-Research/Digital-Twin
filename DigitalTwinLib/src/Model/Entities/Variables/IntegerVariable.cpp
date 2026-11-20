//
// Created by herzog on 01.09.26.
//

#include "IntegerVariable.h"

#include <iostream>
#include <limits>

#include "../../State/ValueCodec.h"

namespace DigitalTwin::Model
{
	IntegerVariable::IntegerVariable(std::string name) : IVariable(name)
	{
		Value = 0;
	}

	IntegerVariable::IntegerVariable(std::string name, const long& value) : IVariable(name)
	{
		Value = value;
	}

	void IntegerVariable::setValue(const long& value)
	{
		Value = value;
		notifyChanged();
		updateLinkedVariables();
	}

	ValueType IntegerVariable::getValueType() const
	{
		return ValueType::Integer;
	}

	VariableValue IntegerVariable::getStateValue() const
	{
		return ValueCodec::encodeInteger(Value);
	}

	bool IntegerVariable::setStateValue(const VariableValue& value)
	{
		int64_t decoded = 0;
		if (!ValueCodec::decodeInteger(value, decoded))
			return false;
		// long is 32 bit on Windows: a stored value that does not fit is a mismatch, not a truncation.
		if (decoded < static_cast<int64_t>(std::numeric_limits<long>::min()) ||
			decoded > static_cast<int64_t>(std::numeric_limits<long>::max()))
			return false;
		setValueWithoutPropagation(static_cast<long>(decoded));
		return true;
	}

	long IntegerVariable::getValue() const
	{
		return Value;
	}

	IVariable* IntegerVariable::copy()
	{
		return new IntegerVariable(Name, Value);
	}

	std::string IntegerVariable::getType()
	{
		return "IntegerVariable";
	}

	void IntegerVariable::updateLinkedVariables()
	{
		for (const auto& variable : LinkedVariables)
		{
			try
			{
				dynamic_cast<IntegerVariable*>(variable)->setValueWithoutPropagation(Value);
			}
			catch (...)
			{
				std::cerr << "Issue wih casting Variable with name: " << variable->getName() << std::endl;
			}
		}
	}

	void IntegerVariable::setValueWithoutPropagation(long value)
	{
		Value = value;
		notifyChanged();
	}

}