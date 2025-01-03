//
// Created by Moritz Herzog on 06.08.24.
//

#ifndef DIGITALTWIN_MQTTBROKERSERVICE_H
#define DIGITALTWIN_MQTTBROKERSERVICE_H

#include <async_mqtt/all.hpp>
#include <boost/program_options.hpp>
#include <boost/format.hpp>

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>

#include "../cpp_physical_twin_communication_global.h"


namespace PHYSICAL_TWIN_COMMUNICATION {
    /**
     *
     */
    class CPPPHYSICALTWINCOMMUNICATION_EXPORT MQTTBrokerService {
    public:
        MQTTBrokerService();

        static void runBroker(uint16_t port, bool& serverStarted);
    private:

    };
}

#endif //DIGITALTWIN_MQTTBROKERSERVICE_H
