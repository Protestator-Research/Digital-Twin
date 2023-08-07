#include "LoggingService.h"

#include <iostream>
#include <filesystem>

#include "../RealTwester.h"

REALTWESTER::LOGGING::LoggingService* REALTWESTER::LOGGING::LoggingService::Myself = nullptr;

REALTWESTER::LOGGING::LoggingService* REALTWESTER::LOGGING::LoggingService::getInstance()
{
	if(Myself==nullptr)
		Myself = new LoggingService();

	return Myself;
}

void REALTWESTER::LOGGING::LoggingService::log(std::string stringToLog)
{
	if (Myself != nullptr)
		Myself->logging(stringToLog);
	else
		throw std::exception("Logging needs to initialized");
}

void REALTWESTER::LOGGING::LoggingService::error(std::string stringToError)
{
	Myself->erroring(stringToError);
}

REALTWESTER::LOGGING::LoggingService::LoggingService()
{
	init();
}

void REALTWESTER::LOGGING::LoggingService::init()
{
	std::cerr << "-----------------------------------------------------------" << std::endl;
	std::cerr << "---------------------  init logger  -----------------------" << std::endl;
	std::cerr << "-----------------------------------------------------------" << std::endl;
}

void REALTWESTER::LOGGING::LoggingService::logging(std::string stringToLog)
{
	std::cout << stringToLog << std::endl;
}

void REALTWESTER::LOGGING::LoggingService::erroring(std::string stringToError)
{
	std::cerr << stringToError << std::endl;
}
