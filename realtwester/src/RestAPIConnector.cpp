#include "RestAPIConnector.h"


namespace DIGITWESTER {
	RestAPIConnector::RestAPIConnector(std::string url, std::string username, std::string password) :
		Username(username),
		Password(password)
	{
		ServerConnection = curl_easy_init();
		if (ServerConnection == NULL)
			throw new std::exception("Curl could not initialize.");

		curl_easy_setopt(ServerConnection, CURLOPT_URL, url);
		curl_easy_setopt(ServerConnection, CURLOPT_FOLLOWLOCATION, 1L);

		auto res = curl_easy_perform(ServerConnection);
		if (res != CURLE_OK)
			throw new std::exception("Could not connect to given Server, is it switched on?");
	}

	RestAPIConnector::~RestAPIConnector()
	{
		curl_easy_cleanup(ServerConnection);
	}

}