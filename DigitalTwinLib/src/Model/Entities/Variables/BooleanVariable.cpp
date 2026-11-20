//
// Created by herzog on 01.09.26.
//

#include "BooleanVariable.h"

#include <iostream>

#include "../../State/ValueCodec.h"

namespace DigitalTwin::Model
{
	BooleanVariable::BooleanVariable(const std::string& name) : IVariable(name)
	{
		Value = false;
	}

	BooleanVariable::BooleanVariable(const std::string& name, bool value) : IVariable(name)
	{
		Value = value;
		updateLinkedVariables();
	}

	bool BooleanVariable::getValue() const
	{
		return Value;
	}

	void BooleanVariable::setValue(const bool& value)
	{
		Value = value;
		notifyChanged();
	}

	ValueType BooleanVariable::getValueType() const
	{
		return ValueType::Boolean;
	}

	VariableValue BooleanVariable::getStateValue() const
	{
		return ValueCodec::encodeBoolean(Value);
	}

	bool BooleanVariable::setStateValue(const VariableValue& value)
	{
		bool decoded = false;
		if (!ValueCodec::decodeBoolean(value, decoded))
			return false;
		setValueWithoutPropagation(decoded);
		return true;
	}

	IVariable* BooleanVariable::copy()
	{
		const auto returnValue = new BooleanVariable(Name, Value);
		return returnValue;
	}

	std::string BooleanVariable::getType()
	{
		return "BooelanVariable";
	}

	void BooleanVariable::updateLinkedVariables()
	{
		for (const auto& variable : LinkedVariables)
		{
			try
			{
				dynamic_cast<BooleanVariable*>(variable)->setValueWithoutPropagation(Value);
			}catch (...)
			{
				std::cerr << "Issue wih casting Variable with name: " << variable->getName() << std::endl;
			}
		}
	}

	void BooleanVariable::setValueWithoutPropagation(bool value)
	{
		Value = value;
		notifyChanged();
	}
} // DigitalTwin::Model