//
// Created by moritzh on 19.05.23.
//

#ifndef ENERGYPROBEDRIVER_DRIVERSESSIONMANAGER_H
#define ENERGYPROBEDRIVER_DRIVERSESSIONMANAGER_H

#include "energy_probe_driver_global.h"
namespace ENERGY_PROBE_DRIVER {
    class DIGITWESTER_EXPORT DriverSessionManager {
    public:
        DriverSessionManager();

        virtual ~DriverSessionManager();

        int getNumberOfProbes();

    private:
        void detectAllProbes();
    };
}

#endif //ENERGYPROBEDRIVER_DRIVERSESSIONMANAGER_H
