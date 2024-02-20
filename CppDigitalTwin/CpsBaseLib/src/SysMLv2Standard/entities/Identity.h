//
// Created by Moritz Herzog on 19.02.24.
//
//---------------------------------------------------------
// Constants, Definitions, Pragmas
//---------------------------------------------------------
#ifndef DIGITALTWIN_IDENTITY_H
#define DIGITALTWIN_IDENTITY_H
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
     * @class Identity
     * @author Moritz Herzog <herzogm@rptu.de>
     * @version 1.0
     * @see
     */
    class Identity : IEntity {
    public:
        /**
         * Constructor if nothing is known.
         */
        Identity();

        /**
         * Copy Constructor
         * @param other Other identity Element
         */
        Identity(Identity &other);

        /**
         * Constructor for the use with a valid ID.
         * @see boost::uuids::uuid
         * @param id The uuid of the object.
         */
        explicit Identity(boost::uuids::uuid id);

        /**
         * Generates Identity object from a JSON string.
         * @param jsonString The json string that allows the generation of the Object.
         * @see nlohmann::json
         */
        explicit Identity(std::string jsonString);

        /**
         *
         * @param identity
         */
        Identity(Identity const &identity);

        /**
         * Destructor
         */
        virtual ~Identity() = default;

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
        Identity &operator=(Identity const &other);


        std::string serializeToJson() override;
    private:
        boost::uuids::uuid Id;
    };
}

#endif //DIGITALTWIN_IDENTITY_H
