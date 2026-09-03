//
// Created by herzog on 01.09.26.
//

#include "PositiveVariable.h"

#include <iostream>

namespace DigitalTwin::Model
{
	PositiveVariable::PositiveVariable(std::string name) : 
	IVariable(name)
	{
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
		updateLinkedVariables();
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
	}
}

