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
#include <curl/curl.h>
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
         * Deleted Constructor, because the Interface is Static
         */
        SysMLAPIImplementation() = delete;

        virtual ~SysMLAPIImplementation() = default;

        /**
         *
         * @param address
         * @return
         */
        static bool connectToServer(std::string address);

        /**
         *
         */
        static void disconnectAndCleanUp();

        static std::string loginUserWithPassword(const std::string& username,const std::string& password);
    private:
        static size_t WriteBufferCallback(char *contents, size_t size, size_t nmemb, void* userp);

        static CURL* ServerConnection;
        static std::string ServerAddress;

    };
}

