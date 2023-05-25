//
// Created by moritzh on 19.05.23.
//

#include "DriverSessionManager.h"

#include "Definitions.h"
#include "Fifo.h"

#include <stdio.h>
#include <stdlib.h>

#ifndef WIN32 
#include <cstring>
#include <unistd.h>
#endif

namespace ENERGY_PROBE_DRIVER {
	DriverSessionManager* DriverSessionManager::PointerToItself = nullptr;

	DriverSessionManager::DriverSessionManager() {
		detectAllProbes();
	}


	void DriverSessionManager::detectAllProbes() {
		for (int i = 0; i < MAX_COUNTERS; i++) {
			mCounterField[i] = 0;
			mCounterChannel[i] = 0;
			mCounterSource[i] = 0;
			mCounterEnabled[i] = false;

		}

		for (int i = 0; i < MAX_CHANNELS; i++) {
			mChannelEnabled[i] = false;
			mResistors[i] = 0;
		}

		mMaxEnabledChannel = -1;

		//TODO Needs to be fittet
		static sem_t senderSem;
        sem_init(&senderSem, 0, 0);
        Fifo* fifo = new Fifo(1 << 15, 1 << 20, &senderSem);
		

		EnergyProbeObject = std::make_unique<ENERGY_PROBE_DRIVER::EnergyProbe>(fifo, this);
	}

	DriverSessionManager* DriverSessionManager::getSessionManager()
	{
		if (PointerToItself == nullptr)
			PointerToItself = new DriverSessionManager();
		return PointerToItself;
	}

	int DriverSessionManager::getNumberOfProbes() {
		return MAX_COUNTERS;
	}

	int DriverSessionManager::getCounterFieldWithIndex(int index)
	{
		return mCounterField[index];
	}

	int DriverSessionManager::getCounterChannelWithIndex(int index)
	{
		return mCounterChannel[index];
	}

	int DriverSessionManager::getCounterSourceWithIndex(int index)
	{
		return mCounterSource[index];
	}

	bool DriverSessionManager::getEnableStateForCounter(int index)
	{
		return mCounterEnabled[index];
	}

	float DriverSessionManager::getSourceScaleFactor(int index)
	{
		return mSourceScaleFactor[index];
	}

	int DriverSessionManager::getShuntResistorWithIndex(int index)
	{
		return mResistors[index];
	}

	int DriverSessionManager::getMaxEnabledChannels()
	{
		return mMaxEnabledChannel;
	}

	void DriverSessionManager::toggleEnableStateForCounter(int index)
	{
		mCounterEnabled[index] = !mCounterEnabled[index];
	}

	void DriverSessionManager::setShuntResistorForChannel(int index, int resistance)
	{
		mResistors[index] = resistance;
	}

	void DriverSessionManager::registerObserver(std::shared_ptr<DataPointObserver> observer)
	{
		Observers.push_back(observer);
	}

	void DriverSessionManager::informObserver(std::shared_ptr<MeasurePoint> measurement)
	{
		for (auto observer : Observers)
			observer->setNewMeasurePoint(measurement);
	}
}
