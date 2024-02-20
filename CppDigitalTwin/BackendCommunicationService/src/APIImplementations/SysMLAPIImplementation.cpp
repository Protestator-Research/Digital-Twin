//
// Created by Moritz Herzog on 14.12.23.
//
//---------------------------------------------------------
// Constants, Definitions, Pragmas
//---------------------------------------------------------

//---------------------------------------------------------
// External Classes
//---------------------------------------------------------
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <BaseFuctions/StringExtention.hpp>

//---------------------------------------------------------
// Internal Classes
//---------------------------------------------------------
#include "SysMLAPIImplementation.h"
#include "../Exeptions/NotEnoughMemoryError.h"
#include "../Exeptions/ConnectionError.h"



namespace BACKEND_COMMUNICATION {

    CURL* SysMLAPIImplementation::ServerConnection = nullptr;
    std::string SysMLAPIImplementation::ServerAddress = "http://cps-agila.cs.uni-kl.de:8080/agila-server";

    std::string SysMLAPIImplementation::loginUserWithPassword(std::string const& username, std::string const& passwod) {
        std::string barrierString;

        CURLcode ServerResult;
        nlohmann::json jsonData;
        jsonData["email"] = username;
        jsonData["password"] = passwod;

        std::string loginData = jsonData.dump();
        std::cout << "Login Data:\r\n" << loginData <<std::endl;

        if(ServerConnection) {
            std::string loginReadBuffer;
            std::string headers;
            curl_easy_setopt(ServerConnection, CURLOPT_URL, (ServerAddress + "/users/login").c_str());
            curl_easy_setopt(ServerConnection, CURLOPT_POSTFIELDS, loginData.c_str());
            curl_easy_setopt(ServerConnection, CURLOPT_WRITEFUNCTION, WriteBufferCallback);
            curl_easy_setopt(ServerConnection, CURLOPT_WRITEDATA, &loginReadBuffer);
            curl_easy_setopt(ServerConnection, CURLOPT_HEADERFUNCTION, WriteBufferCallback);
            curl_easy_setopt(ServerConnection, CURLOPT_HEADERDATA, &headers);

            ServerResult = curl_easy_perform(ServerConnection);
            if(ServerResult == CURLE_OK) {
                std::cout<<"Successfull Answer:"<<std::endl<<loginReadBuffer<<std::endl;
                std::cout<<"Headers:"<<std::endl<<headers<<std::endl;

                auto splittedAnswer = CPSBASELIB::STD_EXTENTION::StringExtention::splitString(loginReadBuffer,' ');
                barrierString = splittedAnswer[2];

            } else {
                throw BACKEND_COMMUNICATION::EXCEPTIONS::ConnectionError(
                        static_cast<BACKEND_COMMUNICATION::EXCEPTIONS::CONNECTION_ERROR_TYPE>(ServerResult));
            }
        }
        return barrierString;
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