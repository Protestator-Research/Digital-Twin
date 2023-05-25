#pragma once

#include "digitwester_global.h"
#include <memory>
#include <include/DriverSessionManager.h>

namespace REALTWESTER {
    class RealTwester {
    public:
        RealTwester();
        virtual ~RealTwester();

        void tryDetectingTheProbe();

        void tryConnectingToAgila();

        void startService();

    private:
        ENERGY_PROBE_DRIVER::DriverSessionManager* SessionManager;
    };
}