//
// Created by Moritz Herzog on 23.02.24.
//
//---------------------------------------------------------
// Constants, Definitions, Pragmas
//---------------------------------------------------------
#ifndef DIGITALTWIN_DIGITALTWIN_H
#define DIGITALTWIN_DIGITALTWIN_H

//---------------------------------------------------------
// External Classes
//---------------------------------------------------------
#include <string>
#include <boost/uuid/uuid.hpp>
#include <vector>

//---------------------------------------------------------
// Internal Classes
//---------------------------------------------------------
#include "Record.h"
#include "DataIdentity.h"

//---------------------------------------------------------
// Forwarding
//---------------------------------------------------------

namespace SysMLv2::Entities {
    /**
     * This class Represents a Digital Twin Object that is sent to the Digital Twin Client and Server.
     * @class DigitalTwin
     * @author Moritz Herzog <herzogm@rptu.de>
     * @version 1.0
     */
    class DigitalTwin : public Record {
    public:
        /**
         *
         */
        DigitalTwin();

        /**
         *
         * @param jsonString
         */
        DigitalTwin(std::string jsonString);

        /**
         *
         * @param other
         * @return
         */
        bool operator==(DigitalTwin const &other);


        std::string serializeToJson() override;
    private:
        DataIdentity ParentProjectId;
        std::vector<DataIdentity> CommitId;

    };
}


#endif //DIGITALTWIN_DIGITALTWIN_H
