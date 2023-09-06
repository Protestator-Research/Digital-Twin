#pragma once

#include "digitwester_global.h"
#include <memory>

#include <DriverSessionManager.h>
// #include <include/DriverSessionManager.h>
#include "Connections/MQTT/MQTTConnectionManager.h"
#include <DataPointObserver.h>


namespace REALTWESTER {
    class RealTwester {
    public:
        RealTwester();
        virtual ~RealTwester();

        void tryDetectingTheProbe();

//        void tryToConnectToMQTTAPI();

        void startService();

    private:
        ENERGY_PROBE_DRIVER::DriverSessionManager* SessionManager;
        CONNECTION::MQTT::MQTTConnectionManager* ConnectionManager;

        std::shared_ptr<ENERGY_PROBE_DRIVER::DataPointObserver> DataPointObserver;
    };
}