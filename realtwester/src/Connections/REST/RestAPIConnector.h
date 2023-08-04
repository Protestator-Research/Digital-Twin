#pragma once

#include "../../digitwester_global.h"
#include "../../Data/DataPoint.h"

#include <string>
#include <memory>
#include <vector>
#include <curl/curl.h>

namespace REALTWESTER {
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
         * Copy Constructor
         * @param other Other RestAPIConnector that is to be copied
         */
        RestAPIConnector(RestAPIConnector& other);

        /**
         * Equals Operator
         * @param other
         * @return
         */
        RestAPIConnector& operator=(RestAPIConnector const &other) = default;

		/**
		 * Using Default destructor.
		 */
		virtual ~RestAPIConnector();

        /**
         *
         */
        void receiveProjects();

        /**
         *
         * @param UID
         */
        void receiveSysMDModelsOfProject(std::string UID);

        /**
         *
         * @param UID
         * @param dataPoint
         */
        void uploadSingleDatapoint(std::string UID, DATA::DataPoint dataPoint);

        /**
         *
         * @param UID
         * @param dataPoints
         */
        void uploadManyDataPoints(std::string UID,std::vector<DATA::DataPoint> dataPoints);
	private:
        void loginIntoTheAgilaBackend();

		std::string Username;
		std::string Password;
        std::string URL;
		CURL* ServerConnection;
	};
}

