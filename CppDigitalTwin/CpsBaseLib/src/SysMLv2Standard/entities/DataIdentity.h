//
// Created by Moritz Herzog on 19.02.24.
//
//---------------------------------------------------------
// Constants, Definitions, Pragmas
//---------------------------------------------------------
#ifndef DIGITALTWIN_DATAIDENTITY_H
#define DIGITALTWIN_DATAIDENTITY_H
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
     * This class represents the Identity of every Object.
     * This object is used, iff the protocol passes only a Identification of another object.
     * @class DataIdentity
     * @author Moritz Herzog <herzogm@rptu.de>
     * @version 1.0
     * @see boost::uuids::uuid
     */
    class DataIdentity : IEntity {
    public:
        /**
         * Constructor if nothing is known.
         */
        DataIdentity();

        /**
         * Copy Constructor
         * @param other Other identity Element
         */
        DataIdentity(DataIdentity &other);

        /**
         * Constructor for the use with a valid ID.
         * @see boost::uuids::uuid
         * @param id The uuid of the object.
         */
        explicit DataIdentity(boost::uuids::uuid id);

        /**
         * Generates Identity object from a JSON string.
         * @param jsonString The json string that allows the generation of the Object.
         * @see nlohmann::json
         */
        explicit DataIdentity(std::string jsonString);

        /**
         * Copy constructor if the identity Object.
         * @param identity The identity to copy.
         */
        DataIdentity(DataIdentity const &identity);

        /**
         * Destructor
         */
        virtual ~DataIdentity() = default;

        /**
         * Returns the interal stored id.
         * @return UUID
         * @see boost::uuids
         */
        boost::uuids::uuid getId() const;

        /**
         * Asignment Operator
         * @param other Other Parameter
         * @return the newly Assigned object
         */
        DataIdentity &operator=(DataIdentity const &other);

        /**
         * Compares the two Identities with each other. The Identities are equal iff the two uuids are equal.
         * @param other Right hand side of the check.
         * @return True iff the identities are equal, else false.
         */
        bool operator==(DataIdentity const &other);

        std::string serializeToJson() override;
    private:
        boost::uuids::uuid Id;
    };
}

#endif //DIGITALTWIN_DATAIDENTITY_H
