#include "MeasurePoint.h"

ENERGY_PROBE_DRIVER::MeasurePoint::MeasurePoint(char* buffer)
{
}

float ENERGY_PROBE_DRIVER::MeasurePoint::getVoltage()
{
	return Voltage;
}
