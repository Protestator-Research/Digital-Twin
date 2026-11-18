//
// Created by Moritz Herzog on 17.01.24.
//

#ifndef DIGITALTWIN_DIGITALTWINMANAGER_H
#define DIGITALTWIN_DIGITALTWINMANAGER_H

#include <map>
#include <mutex>
#include <string>
#include <vector>
#include <boost/uuid/uuid.hpp>

#include <memory>
#include <utility>

#include "cpp_digital_twin_lib_global.h"
#include <Services/IMqttClientService.h>

namespace BACKEND_COMMUNICATION {
    class CommunicationService;
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

namespace DigitalTwin::Communication {
    class ValueEntity;
}

namespace DigitalTwin {
    /**
     *
     */
    class CPPDIGITALTWINLIB_EXPORT DigitalTwinManager {
    public:
        DigitalTwinManager() = delete;
        explicit DigitalTwinManager(BACKEND_COMMUNICATION::CommunicationService* communicationService, DigitalTwin::Communication::IMqttClientService* clientService,  bool isClient = true);
        virtual ~DigitalTwinManager();

        void downloadDigitalTwin(boost::uuids::uuid projectId, boost::uuids::uuid digitalTwinId);

        DigitalTwin::Model::DigitalTwinModel* addDigitalTwinAndCreateModel(std::shared_ptr<SysMLv2::REST::DigitalTwin> digitalTwin);

        std::vector<std::shared_ptr<KerML::Entities::Element>> downloadDigitalTwinModel(boost::uuids::uuid projectId, boost::uuids::uuid commitId);

    private:

        void generateMQTTInterface(Model::DigitalTwinModel* digitalTwin);
        void generateMQTTInterface(const std::string& baseName, const std::pair<std::string, std::shared_ptr<DigitalTwin::Communication::ValueEntity>>& element);
        BACKEND_COMMUNICATION::CommunicationService* BackendCommunicationService;
        DigitalTwin::Communication::IMqttClientService* ClientService;

        // Protects DigitalTwinModelMap, downloads for different twins may run in parallel.
        std::mutex DigitalTwinModelMapMutex;
        std::map<boost::uuids::uuid, Model::DigitalTwinModel*> DigitalTwinModelMap;

        bool IsClient = true;
    };
}

#endif //DIGITALTWIN_DIGITALTWINMANAGER_H
