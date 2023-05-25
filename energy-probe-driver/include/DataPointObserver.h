#pragma once

#include <functional>
#include <memory>

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
		DataPointObserver(std::function<void(std::shared_ptr<MeasurePoint>)> functionToCall);

		void setNewMeasurePoint(std::shared_ptr<MeasurePoint> measurement);

	private:
		std::function<void(std::shared_ptr<MeasurePoint>)> FunctionToCall;
	};
}
