#include <iostream>
#include "RealTwester.h"

#include <stdexcept>
#include <chrono>
#include <thread>

#include "Logging/LoggingService.h"


namespace REALTWESTER {
    RealTwester::RealTwester() {
        LOGGING::LoggingService::getInstance();
    	SessionManager = ENERGY_PROBE_DRIVER::DriverSessionManager::getSessionManager();
        ConnectionManager = new CONNECTION::MQTT::MQTTConnectionManager();
        DataPointObserver = std::make_shared<ENERGY_PROBE_DRIVER::DataPointObserver>([](std::shared_ptr<ENERGY_PROBE_DRIVER::MeasurePoint> dataPoint){
            std::cout<<"Voltage: "<<dataPoint->getVoltage()<<std::endl;
        });
    }

    RealTwester::~RealTwester()
    {
        delete SessionManager;
    }

    void RealTwester::tryDetectingTheProbe() {
        SessionManager->enableChannel(0);
        SessionManager->setShuntResistorForChannel(0,20);

        SessionManager->enableChannel(1);
        SessionManager->setShuntResistorForChannel(1,30);

        SessionManager->compileData();

        SessionManager->detectAndCreateEnergyProbe();

        SessionManager->registerObserver(DataPointObserver);

        if (SessionManager->getNumberOfProbes() < 2) {
            throw new std::length_error("Not enough Probes detected!");
        }

    }

    void RealTwester::startService() {
        SessionManager->startGatheringData();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        SessionManager->stopGatheringData();
    }
}
