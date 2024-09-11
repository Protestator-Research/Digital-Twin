//
// Created by Moritz Herzog on 07.08.24.
//

#ifndef DIGITALTWIN_DIGITALTWINENTITY_H
#define DIGITALTWIN_DIGITALTWINENTITY_H

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <string>

#include "../cpp_physical_twin_communication_global.h"

namespace PHYSICAL_TWIN_COMMUNICATION {
    class CPPPHYSICALTWINCOMMUNICATION_EXPORT DigitalTwinEntity {
    public:
        DigitalTwinEntity()=default;
        DigitalTwinEntity(boost::uuids::uuid digitalTwinId, boost::uuids::uuid projectId);
        explicit DigitalTwinEntity(std::string json);

        virtual ~DigitalTwinEntity() = default;

        std::string serialize();

        boost::uuids::uuid digitalTwinId() const;
        boost::uuids::uuid projectId() const;

    private:
        boost::uuids::uuid DigitalTwinId = boost::uuids::random_generator()();
        boost::uuids::uuid ProjectId = boost::uuids::random_generator()();
    };
}

#endif //DIGITALTWIN_DIGITALTWINENTITY_H
