//
// Created by Moritz Herzog on 17.01.24.
//

#ifndef DIGITALTWIN_DIGITALTWINMANAGER_H
#define DIGITALTWIN_DIGITALTWINMANAGER_H

#include <map>
#include <string>
#include <boost/uuid/uuid.hpp>

namespace BACKEND_COMMUNICATION {
    class CommunicationService;
}

namespace SysMLv2::Entities {
    class DigitalTwin;
}

namespace DigitalTwin::Model {
    class DigitalTwinModel;
}


namespace DigitalTwin {
    class DigitalTwinManager {
    public:
        DigitalTwinManager() = delete;
        explicit DigitalTwinManager(BACKEND_COMMUNICATION::CommunicationService* communicationService);
        virtual ~DigitalTwinManager() = default;

        void downloadDigitalTwin(boost::uuids::uuid projectId, boost::uuids::uuid digitalTwinId);

    private:
        BACKEND_COMMUNICATION::CommunicationService* BackendCommunicationService;
        std::map<boost::uuids::uuid, Model::DigitalTwinModel> DigitalTwinModelMap;

    };
}

#endif //DIGITALTWIN_DIGITALTWINMANAGER_H
