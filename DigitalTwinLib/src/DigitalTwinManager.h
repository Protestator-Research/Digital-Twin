//
// Created by Moritz Herzog on 17.01.24.
//

#ifndef DIGITALTWIN_DIGITALTWINMANAGER_H
#define DIGITALTWIN_DIGITALTWINMANAGER_H

#include <map>
#include <string>
#include <vector>
#include <boost/uuid/uuid.hpp>

#include <memory>

#include "cpp_digital_twin_lib_global.h"

namespace BACKEND_COMMUNICATION {
    class CommunicationService;
}

namespace PHYSICAL_TWIN_COMMUNICATION {
    class MqttClientService;
}

namespace SysMLv2::REST {
    class DigitalTwin;
}

namespace KerML::Entities
{
    class Element;
}

namespace DigitalTwin::Model {
    class DigitalTwinModel;
}

namespace DigitalTwin {
    /**
     *
     */
    class CPPDIGITALTWINLIB_EXPORT DigitalTwinManager {
    public:
        DigitalTwinManager() = delete;
        explicit DigitalTwinManager(BACKEND_COMMUNICATION::CommunicationService* communicationService, PHYSICAL_TWIN_COMMUNICATION::MqttClientService* clientService,  bool isClient = true);
        virtual ~DigitalTwinManager();

        void downloadDigitalTwin(boost::uuids::uuid projectId, boost::uuids::uuid digitalTwinId);

        DigitalTwin::Model::DigitalTwinModel* addDigitalTwinAndCreateModel(std::shared_ptr<SysMLv2::REST::DigitalTwin> digitalTwin);

        std::vector<std::shared_ptr<KerML::Entities::Element>> downloadDigitalTwinModel(boost::uuids::uuid projectId, boost::uuids::uuid commitId);

    private:
        void generateMQTTInterface(Model::DigitalTwinModel* digitalTwin);
        BACKEND_COMMUNICATION::CommunicationService* BackendCommunicationService;
        PHYSICAL_TWIN_COMMUNICATION::MqttClientService* ClientService;

        std::map<boost::uuids::uuid, Model::DigitalTwinModel*> DigitalTwinModelMap;

        bool IsClient = true;
    };
}

#endif //DIGITALTWIN_DIGITALTWINMANAGER_H
