//
// Created by herzog on 01.09.26.
//

#include "RealVariable.h"

#include <iostream>

namespace DigitalTwin::Model
{
	RealVariable::RealVariable(std::string name) :
	IVariable(name)
	{
		Value = 0.0;
	}

	RealVariable::RealVariable(std::string name, double value) : 
	IVariable( name )
	{
		Value = value;
	}

	IVariable* RealVariable::copy()
	{
		return new RealVariable(Name, Value);
	}

	std::string RealVariable::getType()
	{
		return "RealVariable";
	}

	double RealVariable::getValue() const
	{
		return Value;
	}

	void RealVariable::setValue(const double& value)
	{
		Value = value;
		updateLinkedVariables();
	}

	void RealVariable::updateLinkedVariables()
	{
		for (const auto& variable : LinkedVariables)
		{
			try
			{
				dynamic_cast<RealVariable*>(variable)->setValueWithoutPropagation(Value);
			}
			catch (...)
			{
				std::cerr << "Issue wih casting Variable with name: " << variable->getName() << std::endl;
			}
		}
	}

	void RealVariable::setValueWithoutPropagation(const double& value)
	{
		Value = value;
	}
}