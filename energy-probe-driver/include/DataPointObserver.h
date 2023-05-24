#pragma once

#include <functional>

#include "energy_probe_driver_global.h"
#include "MeasurePoint.h"

namespace ENERGY_PROBE_DRIVER
{
	class DIGITWESTER_EXPORT DataPointObserver
	{
	public:
		/**
		 * \brief Default Constructor => Deleted
		 */
		DataPointObserver() = delete;
		/**
		 * \brief Constuctor to setup functionality of DataPoint Observer
		 * \param functionToCall
		 */
		DataPointObserver(std::function<void(MeasurePoint)> functionToCall);

		void setNewMeasurePoint(MeasurePoint measurement);

	private:
		std::function<void(MeasurePoint)> FunctionToCall;
	};
}
