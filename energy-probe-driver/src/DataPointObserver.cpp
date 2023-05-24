#include "DataPointObserver.h"

namespace ENERGY_PROBE_DRIVER {
	DataPointObserver::DataPointObserver(std::function<void(MeasurePoint)> functionToCall)
	{
		FunctionToCall = functionToCall;
	}

	void DataPointObserver::setNewMeasurePoint(MeasurePoint measurement)
	{
		FunctionToCall(measurement);
	}
}
