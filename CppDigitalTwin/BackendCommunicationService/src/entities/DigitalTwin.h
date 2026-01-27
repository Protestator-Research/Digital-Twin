//
// Created by Moritz Herzog on 22.07.25.
//

#ifndef DIGITALTWIN_H
#define DIGITALTWIN_H

#include <sysmlv2/rest/entities/Record.h>
#include <boost/uuid/uuid.hpp>

#include "../cpp_backend_communication_global.h"

namespace SysMLv2::REST {

    /**
        * This class Represents a Digital Twin Object that is sent to the Digital Twin Client and Server.
        * @class DigitalTwin
        * @author Moritz Herzog <herzogm@rptu.de>
        * @version 1.0
        * @see Record
        */
    class CPSBACKENDCOMMUNICATION_EXPORT DigitalTwin : public Record {
    public:
        /**
         * Default Constructor
         * Is deleted, because Record has a deleted default constructor.
         */
        DigitalTwin() = delete;

        /**
         * Constructor that is used to parse the digital twin data or to set only the name of the DT.
         * @param jsonString The given string.
         */
        DigitalTwin(std::string jsonString);

        /**
         *
         * @param name
         * @param connectedElements
         * @param commitId
         */
        DigitalTwin(std::string name, std::vector<boost::uuids::uuid> connectedElements, boost::uuids::uuid commitId);

        /**
         * Destructor.
         */
        ~DigitalTwin() = default;

        /**
         * Checks the Equality of the digital twin.
         * @param other The other digital twn to check.
         * @return True if the digital twins are equal.
         */
        bool operator==(DigitalTwin const &other);

        /**
         * Gives Access to the commit Id.
         * @return 
         */
        boost::uuids::uuid commitId() const;

        boost::uuids::uuid parentProjectId() const;

        std::string serializeToJson() override;

        std::vector<boost::uuids::uuid> getConnectedModels();


    private:
        boost::uuids::uuid ParentProjectId;
        boost::uuids::uuid CommitId;
        std::vector<boost::uuids::uuid> ConnectedModels;

    };

} // SysMLv2::REST

#endif //DIGITALTWIN_H
