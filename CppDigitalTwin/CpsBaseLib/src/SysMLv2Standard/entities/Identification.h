//
// Created by Moritz Herzog on 05.09.24.
//
//---------------------------------------------------------
// Constants, Definitions, Pragmas
//---------------------------------------------------------
#ifndef DIGITALTWIN_IDENTIFICATION_H
#define DIGITALTWIN_IDENTIFICATION_H
//---------------------------------------------------------
// External Classes
//---------------------------------------------------------
#include <boost/uuid/uuid.hpp>
//---------------------------------------------------------
// Internal Classes
//---------------------------------------------------------
#include "IEntity.h"
//---------------------------------------------------------
// Forwarding
//---------------------------------------------------------

namespace SysMLv2::Entities {
    /**
     * Defines the Identification between Objects of the REST API. This not in accordance with the SysML standardization,
     * but needed for the compatibility to the AGILA Backend.
     * @version 1.0
     * @author Moritz Herzog <herzog.moritz@rptu.de>
     * @class Identification
     */
    class Identification : public IEntity {
    public:
        /**
         * CTOR deleted
         */
        Identification() = delete;

        /**
         * Builds the Idenfication object from the uuid
         * @param id The uuid of the object
         * @see boost::uuids::uuid
         */
        explicit Identification(boost::uuids::uuid id);

        /**
         * C-Tor for the parsing of the Identification object.
         * @param JSONstring JSON string of the object.
         */
        explicit Identification(std::string JSONstring);

        /**
         * D-Tor
         */
        ~Identification() = default;

        /**
         * Returns the Id of the given Object.
         * @return Id of the object as uuid
         * @see boost::uuids::uuid
         */
        boost::uuids::uuid getID();

        std::string serializeToJson() override;

    private:
        boost::uuids::uuid Id;
    };
}

#endif //DIGITALTWIN_IDENTIFICATION_H
