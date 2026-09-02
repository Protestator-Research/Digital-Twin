//
// Created by herzog on 01.09.26.
//

#include "StringVariable.h"

#include <iostream>

namespace DigitalTwin::Model
{
	StringVariable::StringVariable(const std::string& name) : IVariable(name)
	{
		Value = "";
	}

	StringVariable::StringVariable(const std::string& name, const std::string& value) : IVariable(name)
	{
		Value = value;
	}

	IVariable* StringVariable::copy()
	{
		return new StringVariable(Name, Value);
	}

	std::string StringVariable::getValue() const
	{
		return Value;
	}

	void StringVariable::setValue(const std::string& value)
	{
		Value = value;
		updateLinkedVariables();
	}

	std::string StringVariable::getType()
	{
		return "StringVariable";
	}

	void StringVariable::updateLinkedVariables()
	{
		for (const auto& variable : LinkedVariables)
		{
			try
			{
				dynamic_cast<StringVariable*>(variable)->setValueWithoutPropagation(Value);
			}catch (...)
			{
				std::cerr << "Issue wih casting Variable with name: " << variable->getName() << std::endl;
			}
		}
	}

	void StringVariable::setValueWithoutPropagation(std::string value)
	{
		Value = value;
	}
}
