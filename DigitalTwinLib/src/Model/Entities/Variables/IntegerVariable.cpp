//
// Created by herzog on 01.09.26.
//

#include "IntegerVariable.h"

#include <iostream>

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
		updateLinkedVariables();
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
	}

}