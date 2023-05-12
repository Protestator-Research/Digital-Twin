#pragma once

#include "digitwester_global.h"
#include <string>
#include <memory>
#include <curl/curl.h>

namespace DIGITWESTER {
	class DIGITWESTER_EXPORT RestAPIConnector
	{
	public:
		/**
		 * Constructor can not be used, because Logindata may vary
		 */
		RestAPIConnector() = delete;

		/**
		 * Constructor. This is the constructor that should be called, because the url and the username and password for the AGILA Backend are given.
		 * @param url The URL of the AGILA server to connect to
		 * @param username The user email that is given by the AGILA Administrator
		 * @param password The password that is given by the AGILA Administrator
		 */
		RestAPIConnector(std::string url, std::string username, std::string password);

		/**
		 * Using Default destructor.
		 */
		virtual ~RestAPIConnector();

	private:
		std::string Username;
		std::string Password;
		CURL* ServerConnection;
	};
}

