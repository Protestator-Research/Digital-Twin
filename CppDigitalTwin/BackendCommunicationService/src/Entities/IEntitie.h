//
// Created by Moritz Herzog on 14.12.23.
//
//---------------------------------------------------------
// Constants, Definitions, Pragmas
//---------------------------------------------------------
#pragma once
//---------------------------------------------------------
// External Classes
//---------------------------------------------------------
#include <string>
//---------------------------------------------------------
// Internal Classes
//---------------------------------------------------------

//---------------------------------------------------------
// Forwarding
//---------------------------------------------------------

namespace BACKEND_COMMUNICATION::ENTITIES {
    /**
     * Baseclass for the Entities. It defines necessary methods that are implemented later on.
     * @author Moritz Herzog <herzogm@rptu.de>
     * @version 1.0
     */
    class IEntitie {
    public:
        /**
         * Generates the REST content for the API call.
         * @return REST content in form of a string.
         */
        virtual std::string generateRestCall() = 0;
    };
}
