#include "MeasurePoint.h"

#include <iostream>

namespace ENERGY_PROBE_DRIVER
{

	MeasurePoint::MeasurePoint(int buffer)
	{
        Voltage = (float)buffer;
	}

	float MeasurePoint::getVoltage()
	{
		return Voltage;
	}
}
