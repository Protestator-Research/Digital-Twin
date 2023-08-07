#pragma once

#include <string>

namespace REALTWESTER::LOGGING
{
	class LoggingService
	{
	public:
		static LoggingService* getInstance();

		static void log(std::string stringToLog);
		
		static void error(std::string stringToError);
	private:
		static LoggingService* Myself;

		LoggingService();

		void init();
		void logging(std::string stringToLog);
		void erroring(std::string stringToError);
	};
}