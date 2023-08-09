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
        int returnValue = 0;
		for(size_t i = 0; i<MAX_CHANNELS;i++){
            if(mChannelEnabled[i])
                returnValue++;
        }
        return returnValue;
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

    void DriverSessionManager::startGatheringData() {
        EnergyProbe1Object->start();
//        EnergyProbe2Object->start();
        ThreadHasToRun = true;
        GetDataThread = std::make_unique<std::thread>(std::bind(&DriverSessionManager::getDataThreadMethod,this));
    }

    void DriverSessionManager::getDataThreadMethod() {
        while(ThreadHasToRun){
            EnergyProbe1Object->processBuffer();
//            EnergyProbe2Object->processBuffer();
        }
    }

    void DriverSessionManager::stopGatheringData() {
        ThreadHasToRun = false;
        EnergyProbe1Object->stop();
//        EnergyProbe2Object->stop();
        GetDataThread->join();
        GetDataThread.reset();
    }

    void DriverSessionManager::enableChannel(size_t index) {
        mChannelEnabled[index] = true;
    }

    void DriverSessionManager::detectAndCreateEnergyProbe() {
        //TODO Needs to be fittet
        static sem_t senderSem;
        sem_init(&senderSem, 0, 0);
        Fifo* fifo = new Fifo(1 << 15, 1 << 20, &senderSem);

        EnergyProbe1Object = std::make_unique<ENERGY_PROBE_DRIVER::EnergyProbe>(fifo, this);
        EnergyProbe1Object->prepareChannels();
        EnergyProbe1Object->init("/dev/ttyACM0");

        EnergyProbe1Object->addTopicToMeasurementValue(0, "DeltaSigma/outputVoltage");
        EnergyProbe1Object->addTopicToMeasurementValue(1, "DeltaSigma/outputVoltage");
        EnergyProbe1Object->addTopicToMeasurementValue(2, "DeltaSigma/integrator1/inputVoltage");

//        EnergyProbe2Object = std::make_unique<ENERGY_PROBE_DRIVER::EnergyProbe>(fifo, this);
//        EnergyProbe2Object->prepareChannels();
//        EnergyProbe2Object->init("/dev/ttyACM1");
    }

    void DriverSessionManager::compileData() {
        static bool compiled = false;

        if (compiled) {
            return;
        }

        int channelsConfigured = 0;
        for (int channel = 0; channel < MAX_CHANNELS; ++channel) {
            // Is a resistor value given for each enabled channel?
            if (mResistors[channel] <= 0) {
                continue;
            }

            mChannelEnabled[channel] = true;

            for (int field = 0; field < 3; ++field) {
                static const int field_num[] = { POWER, CURRENT, VOLTAGE };
                static const int field_source[] = { 0, 2, 1 };

                const int index = 3 * channelsConfigured + field;
                mCounterChannel[index] = channel;
                mCounterField[index] = field_num[field];
//                mCounterDaqCh[index][0] = '\0';
                mCounterEnabled[index] = true;

                // Determine sources
                // Source always in the following order, as per energy meter device:
                // ch0 pwr, ch0 volt, ch0 curr, ch1 pwr, ch1 volt, ch1 curr, ch2 pwr, etc.
                mCounterSource[index] = 3 * channelsConfigured + field_source[field];
                if (mCounterField[index] == POWER || mCounterField[index] == CURRENT) {
                    mSourceScaleFactor[mCounterSource[index]] = 100 / (float) mResistors[channel];
                }
                else {
                    mSourceScaleFactor[mCounterSource[index]] = 1;
                }
            }

            ++channelsConfigured;

            if (channel > mMaxEnabledChannel) {
                mMaxEnabledChannel = channel;
            }
        }

        if (mMaxEnabledChannel < 0) {
            printf("No channels enabled, please ensure resistance values are set \r\n");
            return;
        }

        compiled = true;
    }
}
