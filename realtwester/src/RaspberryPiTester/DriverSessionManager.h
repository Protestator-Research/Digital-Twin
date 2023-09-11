#pragma once

#include "SerialPort.hpp"
#include <functional>
#include <memory>
#include "DataPointObserver.h"

namespace REALTWESTER::RASPI {
    class DriverSessionManager {
    public:
        DriverSessionManager();
        virtual ~DriverSessionManager();

        void registerObserver(std::shared_ptr<ENERGY_PROBE_DRIVER::DataPointObserver> observer);
        void startGatheringData();

    private:
        mn::CppLinuxSerial::SerialPort *serialPort;
        std::shared_ptr<ENERGY_PROBE_DRIVER::DataPointObserver> dataObserver;

        
    };
}