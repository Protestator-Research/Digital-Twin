#pragma once

#include <string>
#include "energy_probe_driver_global.h"

namespace ENERGY_PROBE_DRIVER {
	class DIGITWESTER_EXPORT MeasurePoint
	{
	public:
		MeasurePoint() = delete;
		MeasurePoint(int buffer, std::string topic);

        virtual ~MeasurePoint() = default;


		float getVoltage();

        std::string getTopic();

	private:
		float Voltage;
        std::string Topic;
	};
}
