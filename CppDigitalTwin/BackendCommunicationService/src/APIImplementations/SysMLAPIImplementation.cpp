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
#include <cstring>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <BaseFuctions/StringExtention.hpp>
#include <SysMLv2Standard/SysMLv2Deserializer.h>

//---------------------------------------------------------
// Internal Classes
//---------------------------------------------------------
#include "SysMLAPIImplementation.h"
#include "../Exeptions/NotEnoughMemoryError.h"
#include "../Exeptions/ConnectionError.h"
#include "Standards/HttpReturnCodes.h"
#include "../Exeptions/HttpException.h"



namespace BACKEND_COMMUNICATION {

    std::string SysMLAPIImplementation::ServerAddress = "http://localhost:8080/agila-server";
    std::string SysMLAPIImplementation::ReturnedHeaderData = "";
    std::string SysMLAPIImplementation::Data = "";
    struct curl_slist *SysMLAPIImplementation::HeaderList = NULL;

    std::string SysMLAPIImplementation::loginUserWithPassword(std::string const& username, std::string const& passwod) {
        std::string barrierString;

        CURLcode ServerResult;
        nlohmann::json jsonData;
        jsonData["email"] = username;
        jsonData["password"] = passwod;


        auto serverConnection = setUpServerConnection("users/login", "", jsonData.dump().c_str());

        ServerResult = curl_easy_perform(serverConnection);
        if (ServerResult == CURLE_OK) {
            auto splittedAnswer = CPSBASELIB::STD_EXTENTION::StringExtention::splitString(Data, ' ');
            barrierString = splittedAnswer[2];
        } else {
            throw BACKEND_COMMUNICATION::EXCEPTIONS::ConnectionError(
                    static_cast<BACKEND_COMMUNICATION::EXCEPTIONS::CONNECTION_ERROR_TYPE>(ServerResult));
        }

        curl_easy_cleanup(serverConnection);
        return barrierString;
    }

    void SysMLAPIImplementation::disconnectAndCleanUp() {

        curl_global_cleanup();
    }

    bool SysMLAPIImplementation::connectToServer(std::string address) {
        curl_global_init(CURL_GLOBAL_ALL);

        ServerAddress = address;

        return true;
    }

    size_t SysMLAPIImplementation::WriteBufferCallback(char *contents, size_t size, size_t nmemb, void* userp){
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    std::vector<SysMLv2::Entities::IEntity*> SysMLAPIImplementation::getAllProjects(std::string barrierString) {
        connectToServer(ServerAddress);
        std::vector<SysMLv2::Entities::IEntity*> returnValue;
        CURLcode ServerResult;

        auto serverConnection = setUpServerConnection("projects", barrierString.c_str());

        ServerResult = curl_easy_perform(serverConnection);

        if (ServerResult == CURLE_OK) {
            long httpResult;
            curl_easy_getinfo(serverConnection, CURLINFO_RESPONSE_CODE, &httpResult);

            if(tryToResolveHTTPError(httpResult, serverConnection)==INTERNAL_STATUS_CODE::SUCCESS){
                std::cout<<"Projects:"<<std::endl<<Data<<std::endl;
                returnValue = SysMLv2::SysMLv2Deserializer::deserializeJsonArray(Data);
            }

        } else {
            throw BACKEND_COMMUNICATION::EXCEPTIONS::ConnectionError(
                    static_cast<BACKEND_COMMUNICATION::EXCEPTIONS::CONNECTION_ERROR_TYPE>(ServerResult));
        }
        curl_slist_free_all(HeaderList);
        curl_easy_cleanup(serverConnection);

        return returnValue;
    }

    CURL *SysMLAPIImplementation::setUpServerConnection(const char* urlAppendix, const char* barrierString, const char* postPayload) {
        ReturnedHeaderData = "";
        Data = "";
        HeaderList = NULL;

        CURL* serverConnection = curl_easy_init();

        if(!serverConnection)
            throw BACKEND_COMMUNICATION::EXCEPTIONS::NotEnoughMemoryError();

        char authorizationHeaderBeginning[] ="Authorization: Bearer ";
        char * authorizationHeader = new char[std::strlen(authorizationHeaderBeginning)+std::strlen(barrierString)+1];
        std::strcpy(authorizationHeader,authorizationHeaderBeginning);
        std::strcat(authorizationHeader,barrierString);

        const char* serverAddress = ServerAddress.c_str();
        char* completeServerAddress = new char[std::strlen(serverAddress)+std::strlen(urlAppendix)+1];
        std::strcpy(completeServerAddress,serverAddress);
        std::strcat(completeServerAddress,urlAppendix);

        if(std::strcmp(barrierString, "")!=0)
            HeaderList = curl_slist_append(HeaderList, authorizationHeader);

        curl_easy_setopt(serverConnection, CURLOPT_URL, completeServerAddress);
        curl_easy_setopt(serverConnection, CURLOPT_WRITEFUNCTION, WriteBufferCallback);
        curl_easy_setopt(serverConnection, CURLOPT_WRITEDATA, &Data);
        curl_easy_setopt(serverConnection, CURLOPT_HEADERFUNCTION, WriteBufferCallback);
        curl_easy_setopt(serverConnection, CURLOPT_HEADERDATA, &ReturnedHeaderData);

        if(std::strcmp(barrierString, "")!=0)
            curl_easy_setopt(serverConnection, CURLOPT_HTTPHEADER, HeaderList);

        if(std::strcmp(postPayload, "")!=0) {
            char* payload = new char[std::strlen(postPayload)+1];
            std::strcpy(payload,postPayload);
            curl_easy_setopt(serverConnection, CURLOPT_POSTFIELDS, payload);
        }

        return serverConnection;
    }

    std::vector<SysMLv2::Entities::IEntity*> SysMLAPIImplementation::getAllDigitalTwinsForProject(std::string projectId, std::string barrierString) {
        connectToServer(ServerAddress);
        std::vector<SysMLv2::Entities::IEntity*> returnValue;
        CURLcode ServerResult;

        std::string urlAppendix = "projects/" + projectId + "/digital-twin/";

        auto serverConnection = setUpServerConnection(urlAppendix.c_str(), barrierString.c_str());

        ServerResult = curl_easy_perform(serverConnection);

        if (ServerResult == CURLE_OK) {
            long httpResult;
            curl_easy_getinfo(serverConnection, CURLINFO_RESPONSE_CODE, &httpResult);

            if(tryToResolveHTTPError(httpResult, serverConnection)==INTERNAL_STATUS_CODE::SUCCESS){
                std::cout<<"Digital Twins for Project "<<projectId<<":"<<std::endl<<Data<<std::endl;
                returnValue = SysMLv2::SysMLv2Deserializer::deserializeJsonArray(Data);
            }

        } else {
            throw BACKEND_COMMUNICATION::EXCEPTIONS::ConnectionError(
                    static_cast<BACKEND_COMMUNICATION::EXCEPTIONS::CONNECTION_ERROR_TYPE>(ServerResult));
        }
        curl_slist_free_all(HeaderList);
        curl_easy_cleanup(serverConnection);

        return returnValue;
    }

    INTERNAL_STATUS_CODE SysMLAPIImplementation::tryToResolveHTTPError(long httpErrorCode, void* instance) {
        if(httpErrorCode>STANDARDS::HTTP::HTTP_PROPRIATARY)
            throw BACKEND_COMMUNICATION::EXCEPTIONS::HTTPException(httpErrorCode);

        switch (httpErrorCode) {
            case STANDARDS::HTTP::HTTP_CONTINUE:
            case STANDARDS::HTTP::HTTP_PROTOCOL_SWITCH:
            case STANDARDS::HTTP::HTTP_PROCESSING:
            case STANDARDS::HTTP::HTTP_EARLY_HINTS:
            case STANDARDS::HTTP::HTTP_OK:
            case STANDARDS::HTTP::HTTP_CREATED:
            case STANDARDS::HTTP::HTTP_ACCEPTED:
            case STANDARDS::HTTP::HTTP_NON_AUTHORITATIVE_INFORMATION:
            case STANDARDS::HTTP::HTTP_NO_CONTENT:
            case STANDARDS::HTTP::HTTP_PARTIAL_CONTENT:
            case STANDARDS::HTTP::HTTP_IM_USED:
                return INTERNAL_STATUS_CODE::SUCCESS;

            case STANDARDS::HTTP::HTTP_MULTI_STATUS:
            case STANDARDS::HTTP::HTTP_RESET_CONTENT:
                return INTERNAL_STATUS_CODE::RE_DO;

            case STANDARDS::HTTP::HTTP_MULTIPLE_CHOICES:
            case STANDARDS::HTTP::HTTP_MOVED_PERMANENTLY:
            case STANDARDS::HTTP::HTTP_FOUND:
            case STANDARDS::HTTP::HTTP_SEE_OTHER:
            case STANDARDS::HTTP::HTTP_TEMPORARY_REDIRECT:
            case STANDARDS::HTTP::HTTP_PERMANENT_REDIRECT:
                curl_easy_getinfo(instance, CURLINFO_REDIRECT_URL, &ServerAddress);
                return INTERNAL_STATUS_CODE::RE_DO;

            case STANDARDS::HTTP::HTTP_NOT_MODIFIED:
                return INTERNAL_STATUS_CODE::SUCCESS;

            case STANDARDS::HTTP::HTTP_USE_PROXY:
            case STANDARDS::HTTP::HTTP_SWITCH_PROXY:
                //TODO Needs more consideration
                break;

            case STANDARDS::HTTP::HTTP_ALREADY_REPORTED:
            case STANDARDS::HTTP::HTTP_BAD_REQUEST:
            case STANDARDS::HTTP::HTTP_UNAUTHORIZED:
            case STANDARDS::HTTP::HTTP_PAYMENT_REQUIRED:
            case STANDARDS::HTTP::HTTP_FORBIDDEN:
            case STANDARDS::HTTP::HTTP_NOT_FOUND:
            case STANDARDS::HTTP::HTTP_METHOD_NOT_ALLOWED:
            case STANDARDS::HTTP::HTTP_NOT_ACCEPTABLE:
            case STANDARDS::HTTP::HTTP_PROXY_AUTHENTICATION_REQUIRED:
            case STANDARDS::HTTP::HTTP_REQUEST_TIMEOUT:
            case STANDARDS::HTTP::HTTP_CONFLICT:
            case STANDARDS::HTTP::HTTP_GONE:
            case STANDARDS::HTTP::HTTP_LENGTH_REQUIRED:
            case STANDARDS::HTTP::HTTP_PRECONDITION_FIELD:
            case STANDARDS::HTTP::HTTP_PAYLOAD_TOO_LARGE:
            case STANDARDS::HTTP::HTTP_URI_TOO_LONG:
            case STANDARDS::HTTP::HTTP_UNSUPPORTED_MEDIA_TYPE:
            case STANDARDS::HTTP::HTTP_RANGE_NOT_SATISFIABLE:
            case STANDARDS::HTTP::HTTP_EXPECTATION_FAILED:
            case STANDARDS::HTTP::HTTP_MISDIRECTED_REQUEST:
            case STANDARDS::HTTP::HTTP_UNPROCESSABLE_ENTITY:
            case STANDARDS::HTTP::HTTP_LOCKED:
            case STANDARDS::HTTP::HTTP_FAILED_DEPENDENCY:
            case STANDARDS::HTTP::HTTP_TOO_EARLY:
            case STANDARDS::HTTP::HTTP_UPGRADE_REQUIRED:
            case STANDARDS::HTTP::HTTP_PRECONDITION_REQUIRED:
            case STANDARDS::HTTP::HTTP_TOO_MANY_REQUESTS:
            case STANDARDS::HTTP::HTTP_REQUEST_HEADER_FIELDS:
            case STANDARDS::HTTP::HTTP_UNAVAILABLE_FOR_LEGAL_REASONS:
            case STANDARDS::HTTP::HTTP_INTERNAL_SERVER_ERROR:
            case STANDARDS::HTTP::HTTP_NOT_IMPLEMENTED:
            case STANDARDS::HTTP::HTTP_BAD_GATEWAY:
            case STANDARDS::HTTP::HTTP_SERVICE_UNAVAILABLE:
            case STANDARDS::HTTP::HTTP_GATEWAY_TIMEOUT:
            case STANDARDS::HTTP::HTTP_VERSION_NOT_SUPPORTED:
            case STANDARDS::HTTP::HTTP_VARIANT_ALSO_NEGOTIATES:
            case STANDARDS::HTTP::HTTP_INSUFFCIENT_STORAGE:
            case STANDARDS::HTTP::HTTP_LOOP_DETECTED:
            case STANDARDS::HTTP::HTTP_BANDWIDTH_LIMIT:
            case STANDARDS::HTTP::HTTP_NOT_EXTENDED:
            case STANDARDS::HTTP::HTTP_NETWORK_AUTHENTICATION_REQUIRED:
                throw BACKEND_COMMUNICATION::EXCEPTIONS::HTTPException(httpErrorCode);
        }

        return INTERNAL_STATUS_CODE::THROW_ERROR;
    }
}