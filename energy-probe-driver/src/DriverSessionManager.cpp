//
// Created by moritzh on 19.05.23.
//

#include "DriverSessionManager.h"

#include "Definitions.h"

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

    void DriverSessionManager::toggleEnableStateForCcounter(int index)
    {
        mCounterEnabled[index] = !mCounterEnabled[index];
    }

    void DriverSessionManager::setShuntResotorForChannel(int index, int resistance)
    {
        mResistors[index] = resistance;
    }
}
