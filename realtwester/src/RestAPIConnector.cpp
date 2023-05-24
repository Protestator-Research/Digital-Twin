#include "RestAPIConnector.h"

#include <exception>
#include <utility>

#include <nlohmann/json.hpp> 

#include "Exceptions/ServerNotReachableException.h"


namespace REALTWESTER {
	RestAPIConnector::RestAPIConnector(std::string url, std::string username, std::string password) :
		Username(std::move(username)),
		Password(std::move(password)),
        URL(std::move(url))
	{
		ServerConnection = curl_easy_init();
		if (ServerConnection == NULL)
            throw new std::bad_alloc();

		curl_easy_setopt(ServerConnection, CURLOPT_URL, URL.c_str());
		curl_easy_setopt(ServerConnection, CURLOPT_FOLLOWLOCATION, 1L);

		auto res = curl_easy_perform(ServerConnection);
		if (res != CURLE_OK)
			throw new EXCEPTIONS::ServerNotReachableException();
	}

	RestAPIConnector::~RestAPIConnector()
	{
		curl_easy_cleanup(ServerConnection);
	}

    RestAPIConnector::RestAPIConnector(RestAPIConnector &other) {
        URL = other.URL;
        Username = other.Username;
        Password = other.Password;
        ServerConnection = other.ServerConnection;
    }

    void RestAPIConnector::loginIntoTheAgilaBackend() {
		nlohmann::json j;
		j["email"] = Username;
		j["password"] = Password;
		curl_easy_setopt(ServerConnection, CURLOPT_URL, (URL + "/users/login"));
		curl_easy_setopt(ServerConnection, CURLOPT_POSTFIELDS, "sessionid=1"); // TODO SessionID von Server Erhalten
//		curl_easy_set
//		auto result =


    }

    void RestAPIConnector::receiveProjects() {

    }

    void RestAPIConnector::receiveSysMDModelsOfProject(std::string UID) {

    }

    void RestAPIConnector::uploadSingleDatapoint(std::string UID, DATA::DataPoint dataPoint) {

    }

    void RestAPIConnector::uploadManyDataPoints(std::string UID, std::vector<DATA::DataPoint> dataPoints) {

    }

}