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
#include <vector>
#include <string>
#include <uuid/uuid.h>
//---------------------------------------------------------
// Internal Classes
//---------------------------------------------------------

//---------------------------------------------------------
// Forwarding
//---------------------------------------------------------

namespace BACKEND_COMMUNICATION {
    /**
     * Implements the Protocol of the SysMLv2 API Standard.
     * @version 1.0
     * @author Moritz Herzog <herzogm@rptu.de>
     */
    class SysMLAPIImplementation {
    public:
        /**
         * Logins in the user and returns the
         * @param username
         * @param passwod
         * @return
         */
        static std::string loginUserWithPassword(std::string username, std::string passwod);
    };
}

