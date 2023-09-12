#pragma once

#include "SerialPort.hpp"
#include <functional>
#include <memory>
#include "DataPointObserver.h"
#include <thread>
#include <mutex>
#include <string>

namespace REALTWESTER::RASPI {
    class DriverSessionManager {
    public:
        DriverSessionManager();
        virtual ~DriverSessionManager();

        void registerObserver(std::shared_ptr<ENERGY_PROBE_DRIVER::DataPointObserver> observer);
        
        void startGatheringData();
        void stopGatheringData();

    private:
        mn::CppLinuxSerial::SerialPort *serialPort;
        std::shared_ptr<ENERGY_PROBE_DRIVER::DataPointObserver> dataObserver;

        std::thread* bufferThread = nullptr;
        std::thread* sendingThread = nullptr;

        std::string bufferString="";

        std::mutex mtx;

        const std::string topics[4] = {
            "DeltaSigma/inputVoltage",
            "DeltaSigma/outputVoltage",
            "DeltaSigma/integrator1/outputVoltage",
            "DeltaSigma/integrator2/outputVoltage"
        };

        bool isNotStopped=true;

        void createThreads();

        void readFromSerialPort();
        void writeToMQTTServer();
    };
}