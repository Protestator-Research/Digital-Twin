#pragma once

#include "digitwester_global.h"

namespace DIGITWESTER_EXPORT DIGITWESTER {
    class Digitwester {
    public:
        Digitwester();
        virtual ~Digitwester()=default;

        void tryDetectingTheProbe();

        void tryConnectingToAgila();

        void startService();

    private:

    };
}