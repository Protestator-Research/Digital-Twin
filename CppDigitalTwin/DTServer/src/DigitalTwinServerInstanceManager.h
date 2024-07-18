//
// Created by Moritz Herzog on 17.01.24.
//

#pragma once

#include <string>
#include <cstdlib>
#include <map>
#include <BECommunicationService.h>
#include <DigitalTwinManager.h>
#include <PTCommunicationService.h>


namespace DIGITAL_TWIN_SERVER {
    /**
     *
     */
    enum ARGUMENTS {
        AGILA_URL,
        AGILA_PORT,
        AGILA_USERNAME,
        AGILA_PASSWORD,
        INSTANCE_MQTT_PORT,
        ARGUMENTS_SIZE
    };

    /**
     * Allows for the
     * @version 1.0
     * @author Moritz Herzog <herzogm@rptu.de>
     */
    class DigitalTwinServerInstanceManager {
    public:
        /**
         * Constructor to
         * @param argc
         * @param argv
         */
        DigitalTwinServerInstanceManager(int argc, char *argv[]);
        /**
         * Generalized Constructor is deleted to allow to change the sessions and Properties of the sessions.
         */
        DigitalTwinServerInstanceManager() = delete;
        virtual ~DigitalTwinServerInstanceManager();

        void createInstance();
        void runInstance();

        void destroyOnError();

        int getRunTimeCode();
    private:
        /**
         *
         */
        const std::string Arguments[ARGUMENTS_SIZE] = {
            "agila.url",
            "agila.port",
            "agila.username",
            "agila.password",
            "instance.mqtt.port"
        };

        /**
         *
         */
        const std::string DefaultValueForArgument[ARGUMENTS_SIZE]{
            "localhost",
            "8080",
            "admin@cps.de",
            "admin",
            "1883"
        };

        void mapInstanceSettingsByArguments(int argc, char *argv[]);

        BACKEND_COMMUNICATION::CommunicationService* BackendCommunicationService = nullptr;
        DigitalTwin::DigitalTwinManager* DigitalTwinManager = nullptr;
        PHYSICAL_TWIN_COMMUNICATION::CommunicationService* PhysicalTwinCommunicationService = nullptr;


        std::map<ARGUMENTS,std::string> ArgumentsMap;

        int ErrorCode = EXIT_SUCCESS;
    };
}