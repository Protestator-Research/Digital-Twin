//
// Created by herzog on 01.09.26.
//

#include "RationalVariable.h"

#include <iostream>

namespace DigitalTwin::Model
{
	RationalVariable::RationalVariable(std::string name) :
	IVariable(name)
	{
		Value = boost::rational<long>();
	}

	RationalVariable::RationalVariable(std::string name, const long& numerator, const long& denumerator) :
	IVariable(name)
	{
		Value = boost::rational<long>(numerator,denumerator);
	}

	RationalVariable::RationalVariable(std::string name, const boost::rational<long>& value) : 
	IVariable(name)
	{
		Value = value;
	}

	IVariable* RationalVariable::copy()
	{
		return new RationalVariable(Name, Value);
	}

	std::string RationalVariable::getType()
	{
		return "RationalVariable";
	}

	void RationalVariable::setValue(const long& numerator, const long& denumerator)
	{
		Value.assign(numerator, denumerator);
	}

	void RationalVariable::setValue(const boost::rational<long>& value)
	{
		Value = value;
		updateLinkedVariables();
	}

	boost::rational<long> RationalVariable::getValue()
	{
		return Value;
	}

	void RationalVariable::updateLinkedVariables()
	{
		for (const auto& variable : LinkedVariables)
		{
			try
			{
				dynamic_cast<RationalVariable*>(variable)->setValueWithoutPropagation(Value);
			}
			catch (...)
			{
				std::cerr << "Issue wih casting Variable with name: " << variable->getName() << std::endl;
			}
		}
	}

	void RationalVariable::setValueWithoutPropagation(const boost::rational<long>& value)
	{
		Value = value;
	}
}