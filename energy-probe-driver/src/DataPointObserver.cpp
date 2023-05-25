#include "DataPointObserver.h"

namespace ENERGY_PROBE_DRIVER {
	DataPointObserver::DataPointObserver(std::function<void(std::shared_ptr<MeasurePoint>)> functionToCall)
	{
		FunctionToCall = functionToCall;
	}

	void DataPointObserver::setNewMeasurePoint(std::shared_ptr<MeasurePoint> measurement)
	{
		FunctionToCall(measurement);
	}
}
