#pragma once

#include "energy_probe_driver_global.h"

namespace ENERGY_PROBE_DRIVER {
	class DIGITWESTER_EXPORT MeasurePoint
	{
	public:
		MeasurePoint() = delete;
		MeasurePoint(int buffer);

		virtual ~MeasurePoint() = default;

		float getVoltage();

	private:
		float Voltage;
	};
}
