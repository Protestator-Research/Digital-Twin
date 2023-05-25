#include "MeasurePoint.h"

#include <iostream>

namespace ENERGY_PROBE_DRIVER
{

	MeasurePoint::MeasurePoint(int buffer)
	{
		std::cout << "Chars: " << buffer << std::endl;
	}

	float MeasurePoint::getVoltage()
	{
		return Voltage;
	}
}
