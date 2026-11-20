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

        /**
         * MQTT callbacks arrive on threads of the MQTT library. Work that changes a model is handed to this
         * dispatcher, which runs it on the thread that owns the models (e.g. the GUI thread). Without one the work
         * runs directly on the calling thread. Set it before the first twin is opened.
         */
        void setCallbackDispatcher(std::function<void(std::function<void()>)> dispatcher);

        /**
         * Replaces the MQTT client, e.g. after a new connection was built with corrected credentials. The opened
         * twins and their models stay. Call it on the thread that owns the models; the caller keeps ownership of
         * both clients and destroys the old one only after this call.
         */
        void setClientService(DigitalTwin::Communication::IMqttClientService* clientService);

        /**
         * Sends the connectToTwin request of every opened twin again. Called after every (re)connect of the MQTT
         * client: requests published while the client was not connected are dropped, and a restarted server has
         * forgotten the twins.
         */
        void announceOpenTwins();

    private:
        void runOnOwningThread(std::function<void()> work);
        std::function<void(std::function<void()>)> CallbackDispatcher;

        void generateMQTTInterface(Model::DigitalTwinModel* digitalTwin);
        void generateMQTTInterface(const std::string& baseName, const std::pair<std::string, std::shared_ptr<DigitalTwin::Communication::ValueEntity>>& element);
        BACKEND_COMMUNICATION::CommunicationService* BackendCommunicationService;
        DigitalTwin::Communication::IMqttClientService* ClientService;

        // Protects DigitalTwinModelMap, downloads for different twins may run in parallel.
        std::mutex DigitalTwinModelMapMutex;
        std::map<boost::uuids::uuid, Model::DigitalTwinModel*> DigitalTwinModelMap;
        // Serialized connectToTwin request per opened twin (see announceOpenTwins), also protected by DigitalTwinModelMapMutex.
        std::map<boost::uuids::uuid, std::string> TwinAnnouncements;

        bool IsClient = true;
    };
}

#endif //DIGITALTWIN_DIGITALTWINMANAGER_H
