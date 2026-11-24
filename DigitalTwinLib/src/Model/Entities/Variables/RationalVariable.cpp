//
// Created by herzog on 01.09.26.
//

#include "RationalVariable.h"

#include <iostream>
#include <limits>

#include "../../State/ValueCodec.h"

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
		notifyChanged();
	}

	void RationalVariable::setValue(const boost::rational<long>& value)
	{
		Value = value;
		notifyChanged();
		updateLinkedVariables();
	}

	ValueType RationalVariable::getValueType() const
	{
		return ValueType::Rational;
	}

	VariableValue RationalVariable::getStateValue() const
	{
		return ValueCodec::encodeRational(Value.numerator(), Value.denominator());
	}

	bool RationalVariable::setStateValue(const VariableValue& value)
	{
		int64_t numerator = 0;
		int64_t denominator = 1;
		if (!ValueCodec::decodeRational(value, numerator, denominator))
			return false;
		const auto longMin = static_cast<int64_t>(std::numeric_limits<long>::min());
		const auto longMax = static_cast<int64_t>(std::numeric_limits<long>::max());
		// A negated minimum is not representable, boost::rational normalises by negating.
		if (numerator <= longMin || numerator > longMax || denominator <= longMin || denominator > longMax)
			return false;
		setValueWithoutPropagation(boost::rational<long>(static_cast<long>(numerator), static_cast<long>(denominator)));
		return true;
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
		notifyChanged();
	}
}