//
// Created by Moritz Herzog on 05.09.24.
//
//---------------------------------------------------------
// Constants, Definitions, Pragmas
//---------------------------------------------------------

//---------------------------------------------------------
// External Classes
//---------------------------------------------------------
#include <curl/curl.h>
#include <string>

//---------------------------------------------------------
// Internal Classes
//---------------------------------------------------------
#include "IAPIInterface.h"

namespace BACKEND_COMMUNICATION {

    void IAPIInterface::disconnectAndCleanUp() {
        curl_global_cleanup();
    }

    size_t IAPIInterface::WriteBufferCallback(char *contents, size_t size, size_t nmemb, void* userp){
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }
}
