#include "MeasurePoint.h"

#include <iostream>

namespace ENERGY_PROBE_DRIVER
{

	MeasurePoint::MeasurePoint(int buffer, std::string topic)
	{
        Voltage = (float)buffer;
        Topic = topic;
	}

	MeasurePoint::MeasurePoint(float value, std::string topic)
	{
        Voltage = value;
        Topic = topic;
	}

	float MeasurePoint::getVoltage()
	{
		return Voltage;
	}

    std::string MeasurePoint::getTopic() {
        return Topic;
    }
}
