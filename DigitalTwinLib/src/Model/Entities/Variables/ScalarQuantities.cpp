//
// Created by herzog on 01.10.26.
//

#include "ScalarQuantities.h"

#include "../../State/ValueCodec.h"

namespace DigitalTwin::Model
{
	// Scalar quantities carry no value yet (createValueEntity treats them as the Real 0). They are reported as such.
	ValueType ScalarQuantities::getValueType() const
	{
		return ValueType::Real;
	}

	VariableValue ScalarQuantities::getStateValue() const
	{
		return ValueCodec::encodeReal(0.0);
	}

	bool ScalarQuantities::setStateValue(const VariableValue&)
	{
		return false;
	}
}
