//
// Created by moritzh on 19.05.23.
//

#ifndef ENERGYPROBEDRIVER_DRIVERSESSIONMANAGER_H
#define ENERGYPROBEDRIVER_DRIVERSESSIONMANAGER_H

#include <memory>
#include <vector>
#include <thread>

#include "Definitions.h"
#include "energy_probe_driver_global.h"
#include "DataPointObserver.h"
#include "EnergyProbe.h"


namespace ENERGY_PROBE_DRIVER {
    class DIGITWESTER_EXPORT DriverSessionManager {
    public:
	    /**
	     * \brief Using Signleton Pattern.
	     * \return The only Instance of the SessionManager
	     */
	    static DriverSessionManager* getSessionManager();
        virtual ~DriverSessionManager() = default;

        int getNumberOfProbes();

		int getCounterFieldWithIndex(int index);
		int getCounterChannelWithIndex(int index);
		int getCounterSourceWithIndex(int index);
		bool getEnableStateForCounter(int index);
		float getSourceScaleFactor(int index);
	    /**
	     * \brief Returns the shunt resistor of the channel in milli Ohm, dependend on the index.
	     * \param index The index of the Channel.
	     * \return Shunt resistor in milli ohm.
	     */
	    int getShuntResistorWithIndex(int index);
		int getMaxEnabledChannels();

		void toggleEnableStateForCounter(int index);
	    /**
	     * \brief Sets the shunt resistor of a specific index in milli ohm
	     * \param index Index of the channel
	     * \param resistance Resistance of the shunt resistor in milli ohm 
	     */
	    void setShuntResistorForChannel(int index, int resistance);

		void registerObserver(std::shared_ptr<DataPointObserver>);
		void informObserver(std::shared_ptr<MeasurePoint> measurement);


        void startGatheringData();
        void stopGatheringData();

        void enableChannel(size_t index);

        void detectAndCreateEnergyProbe();

        void compileData();
    private:
		static DriverSessionManager* PointerToItself;
		DriverSessionManager();
        void detectAllProbes();

        void getDataThreadMethod();
			   
		// Counters
		// one of power, voltage, or current
		int mCounterField[MAX_COUNTERS];
		// channel 0, 1, or 2
		int mCounterChannel[MAX_COUNTERS];
		// which source of data emitted from the energy probe, 0-8
		int mCounterSource[MAX_COUNTERS];
		// DAQ Channel, such as 'ai1', 'ai2', etc.
		//char mCounterDaqCh[MAX_COUNTERS][MAX_STRING_LEN];
		// whether this counter is enabled
		bool mCounterEnabled[MAX_COUNTERS];

		// scale factor based on a 0.1 ohm shunt resistor
		float mSourceScaleFactor[MAX_FIELDS];

		// whether this channel is enabled
		bool mChannelEnabled[MAX_CHANNELS];
		// shunt resistor
		int mResistors[MAX_CHANNELS];


        int mMaxEnabledChannel = -1;

		std::vector<std::shared_ptr<DataPointObserver>> Observers;
		std::unique_ptr<EnergyProbe> EnergyProbeObject;

        std::unique_ptr<std::thread> GetDataThread;
        bool ThreadHasToRun = false;
    };
}

#endif //ENERGYPROBEDRIVER_DRIVERSESSIONMANAGER_H
