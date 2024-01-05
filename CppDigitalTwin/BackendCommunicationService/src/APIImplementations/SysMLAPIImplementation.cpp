//
// Created by Moritz Herzog on 14.12.23.
//

#include "SysMLAPIImplementation.h"

#include "../Exeptions/NotEnoughMemoryError.h"

#include <fstream>
#include <nlohmann/json.hpp>

namespace BACKEND_COMMUNICATION {
    CURL* SysMLAPIImplementation::ServerConnection = nullptr;

    std::string SysMLAPIImplementation::loginUserWithPassword(std::string username, std::string passwod) {
        CURLcode ServerResult;
        nlohmann::json data;
        data["email"] = username;
        data["password"] = passwod;

        if(ServerConnection) {
            curl_easy_setopt(ServerConnection, CURLOPT_URL, (ServerAddress + "/users/login").c_str());
            curl_easy_setopt(ServerConnection, CURLOPT_POSTFIELDS, data.dump().c_str());
            ServerResult = curl_easy_perform(ServerConnection);
            if(ServerResult == CURLE_OK) {

            }
        }
    }

    void SysMLAPIImplementation::disconnectAndCleanUp() {
        if(ServerConnection) {
            curl_easy_cleanup(ServerConnection);
        }
        curl_global_cleanup();
    }

    bool SysMLAPIImplementation::connectToServer(std::string address) {
        curl_global_init(CURL_GLOBAL_ALL);
        ServerConnection = curl_easy_init();

        if(!ServerConnection)
            throw BACKEND_COMMUNICATION::EXCEPTIONS::NotEnoughMemoryError();

        ServerAddress = address;

        return true;
    }
}