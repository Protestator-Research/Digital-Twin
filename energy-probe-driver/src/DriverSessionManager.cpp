//
// Created by moritzh on 19.05.23.
//

#include "DriverSessionManager.h"

#include "Definitions.h"
#include "Fifo.h"

#include <cstring>
#include <unistd.h>

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
		FILE* binfile = NULL;
		char* outputPath = (char*)malloc(CAIMAN_PATH_MAX + 1);
		char* binaryPath = (char*)malloc(CAIMAN_PATH_MAX + 1);
		strncpy(outputPath, "./test", CAIMAN_PATH_MAX);
		Fifo* fifo = NULL;
		static sem_t senderSem, senderThreadStarted;

		outputPath[CAIMAN_PATH_MAX - 1] = 0; // strncpy does not guarantee a null-terminated string

	// Ensure the path ends with a path separator
		int n = strlen(outputPath);
		if (outputPath[n - 1] != '/' && outputPath[n - 1] != '\\') {
			strncat(outputPath, "/", CAIMAN_PATH_MAX - n - 1);
		}

		// Set up warnings file
		snprintf(binaryPath, CAIMAN_PATH_MAX, "%swarnings.xml", outputPath);
		unlink(binaryPath);

		// Create a string representing the path to the binary output file and open it
		snprintf(binaryPath, CAIMAN_PATH_MAX, "%s0000000000", outputPath);
		if ((binfile = fopen(binaryPath, "wb")) == 0) {
			printf("Unable to open output file: %s0000000000\nPlease check write permissions on this file. \r\n", outputPath);
		}
		else {
			if (sem_init(&senderSem, 0, 0) || sem_init(&senderThreadStarted, 0, 0)) {
				printf("sem_init() failed");
			}
			fifo = new Fifo(1 << 15, 1 << 20, &senderSem);
			//THREAD_CREATE(senderThreadID, senderThread);
			//if (!senderThreadID) {
			//	logg.logError("Failed to create sender thread");
			//	handleException();
			//}
		}
		fifo = new Fifo(1 << 15, 1 << 20, &senderSem);
		EnergyProbe = std::make_unique<ENERGY_PROBE_DRIVER::EnergyProbe>(outputPath, binfile, fifo);
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
