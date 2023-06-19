#include <iostream>
#include "RealTwester.h"

#include <stdexcept>
#include <chrono>
#include <thread>

namespace REALTWESTER {
    RealTwester::RealTwester() {
        SessionManager = ENERGY_PROBE_DRIVER::DriverSessionManager::getSessionManager();
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

        if (SessionManager->getNumberOfProbes() < 0) {
            throw new std::length_error("Not enough Probes detected!");
        }

    }

    void RealTwester::tryConnectingToAgila() {

    }

    void RealTwester::startService() {
        SessionManager->startGatheringData();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        SessionManager->stopGatheringData();
    }
}