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
#include "StatusCode.h"
//---------------------------------------------------------
// Forwarding
//---------------------------------------------------------
namespace SysMLv2::Entities {
    class IEntity;
}

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

        /**
         *
         */
        static std::vector<SysMLv2::Entities::IEntity*> getAllProjects(std::string barrierString);

        static std::string loginUserWithPassword(const std::string& username,const std::string& password);
    private:
        static CURL* setUpServerConnection(const char* urlAppendix, const char* barrierString="", const char* postPayload="");

        static size_t WriteBufferCallback(char *contents, size_t size, size_t nmemb, void* userp);

        static INTERNAL_STATUS_CODE tryToResolveHTTPError(long httpErrorCode, void* instance);

        static std::string ServerAddress;
        static std::string ReturnedHeaderData;
        static std::string Data;
        static struct curl_slist *HeaderList;
    };
}

