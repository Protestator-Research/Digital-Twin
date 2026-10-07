//
// Created by Moritz Herzog on 17.01.24.
//

#pragma once

#include <string>
#include <cstdlib>
#include <map>
#include <BECommunicationService.h>
#include <DigitalTwinManager.h>
#include <Services/MqttClientService.h>
#include "MqttBrokerService.h"


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
        INSTANCE_MQTT_CERT_CHAIN,
        INSTANCE_MQTT_CERT_PRIV,
        INSTANCE_CONFIG_FILE_PATH,
        ARGUMENTS_SIZE
    };

    /**
     * Allows for the Digital Twin Server to manage its instance.
     * @version 1.0
     * @author Moritz Herzog <herzogm@rptu.de>
     */
    class DigitalTwinServerInstanceManager {
    public:
        /**
         * Constructor to create the Instace Manager.
         * @param argc The program argument counter
         * @param argv The program argument list
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
         * Mapps the elements from the ARGUMENTS Enum to the strings that indexes the the individual argument texts
         */
        const std::string Arguments[ARGUMENTS_SIZE] = {
            "sysml.url",
            "sysml.port",
            "sysml.username",
            "sysml.password",
            "instance.mqtt.port",
			"instance.mqtt.cert_chain",
            "instance.mqtt.cert_private_key",
            "instance.config"
        };

        /**
         * The default values for the Arguments map
         */
        const std::string DefaultValueForArgument[ARGUMENTS_SIZE]{
            "localhost",
            "8088",
            "admin",
            "admin",
            "1883",
			"",
			"",
            ""
        };

        void mapInstanceSettingsByArguments(int argc, char *argv[]);
        void openConfigFileIfExist();

        void createDTTopicAndCallback();

        BACKEND_COMMUNICATION::CommunicationService* BackendCommunicationService = nullptr;
        DigitalTwin::DigitalTwinManager* DigitalTwinManager = nullptr;
        DigitalTwin::Communication::MqttClientService* ClientService = nullptr;
        MQTTBrokerService* BrokerService = nullptr;

        std::vector<SysMLv2::REST::Project*> Projects;
        std::map<ARGUMENTS,std::string> ArgumentsMap;


        int ErrorCode = EXIT_SUCCESS;
    };
}