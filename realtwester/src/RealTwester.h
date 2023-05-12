#pragma once

#include "digitwester_global.h"

namespace REALTWESTER {
    class DIGITWESTER_EXPORT RealTwester {
    public:
        RealTwester();
        virtual ~RealTwester()=default;

        void tryDetectingTheProbe();

        void tryConnectingToAgila();

        void startService();

    private:

    };
}