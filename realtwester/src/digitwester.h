#pragma once

#include "digitwester_global.h"

namespace DIGITWESTER {
    class DIGITWESTER_EXPORT Digitwester {
    public:
        Digitwester();
        virtual ~Digitwester()=default;

        void tryDetectingTheProbe();

        void tryConnectingToAgila();

        void startService();

    private:

    };
}