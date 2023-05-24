#include <iostream>
#include "RealTwester.h"

#include <stdexcept>

namespace REALTWESTER {
    RealTwester::RealTwester() {
        SessionManager = ENERGY_PROBE_DRIVER::DriverSessionManager::getSessionManager();
    }

    RealTwester::~RealTwester()
    {
        delete SessionManager;
    }

    void RealTwester::tryDetectingTheProbe() {
        if (SessionManager->getNumberOfProbes() < 0)
            throw new std::length_error("Not enough Probes detected!");
    }

    void RealTwester::tryConnectingToAgila() {

    }

    void RealTwester::startService() {

    }
}