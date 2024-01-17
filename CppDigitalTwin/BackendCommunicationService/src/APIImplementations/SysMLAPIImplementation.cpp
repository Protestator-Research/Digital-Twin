//
// Created by Moritz Herzog on 14.12.23.
//

#include "SysMLAPIImplementation.h"

#include "../Exeptions/NotEnoughMemoryError.h"
#include "../Exeptions/ConnectionError.h"

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

namespace BACKEND_COMMUNICATION {
    CURL* SysMLAPIImplementation::ServerConnection = nullptr;
    std::string SysMLAPIImplementation::ServerAddress = "https://cps-agila.cs.uni-kl.de/agila-server";

    std::string SysMLAPIImplementation::loginUserWithPassword(std::string username, std::string passwod) {
        CURLcode ServerResult;
        nlohmann::json data;
        data["email"] = username;
        data["password"] = passwod;

        if(ServerConnection) {
            std::string loginReadBuffer;

            curl_easy_setopt(ServerConnection, CURLOPT_URL, (ServerAddress + "/users/login").c_str());
            curl_easy_setopt(ServerConnection, CURLOPT_POSTFIELDS, data.dump().c_str());
            curl_easy_setopt(ServerConnection, CURLOPT_WRITEFUNCTION, WriteBufferCallback);
            curl_easy_setopt(ServerConnection, CURLOPT_WRITEDATA, &loginReadBuffer);

            ServerResult = curl_easy_perform(ServerConnection);
            if(ServerResult == CURLE_OK) {
                std::cout<<"Successfull Answer:"<<std::endl<<loginReadBuffer;
            } else {
                throw new BACKEND_COMMUNICATION::EXCEPTIONS::ConnectionError(
                        static_cast<BACKEND_COMMUNICATION::EXCEPTIONS::CONNECTION_ERROR_TYPE>(ServerResult));
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

    size_t SysMLAPIImplementation::WriteBufferCallback(char *contents, size_t size, size_t nmemb, void* userp){
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }
}