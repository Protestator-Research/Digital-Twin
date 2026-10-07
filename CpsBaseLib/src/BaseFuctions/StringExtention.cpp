//
// Created by Moritz Herzog on 19.02.24.
//
//---------------------------------------------------------
// Constants, Definitions, Pragmas
//---------------------------------------------------------

//---------------------------------------------------------
// External Classes
//---------------------------------------------------------
#include <sstream>
#include <cctype>
#include <algorithm>
//---------------------------------------------------------
// Internal Classes
//---------------------------------------------------------
#include "StringExtention.hpp"


namespace CPSBASELIB::STD_EXTENTION {
    std::vector<std::string> STD_EXTENTION::StringExtention::splitString(std::string contentString, char delimiter) {
        std::vector<std::string> returnValue;
        std::istringstream stream(contentString);
        std::string line;
        while (getline(stream, line, delimiter)) {
            returnValue.push_back(line);
        }

        return returnValue;
    }

    std::string StringExtention::toLower(std::string string) {
        std::transform(string.begin(), string.end(), string.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        return string;
    }

    std::string StringExtention::timepointToString(std::chrono::time_point<std::chrono::system_clock> timepoint)
    {
        auto ms = std::chrono::floor<std::chrono::milliseconds>(timepoint);

        return std::format("{:%FT%TZ}", ms);
    }

    std::chrono::time_point<std::chrono::system_clock> StringExtention::timepointFromString(std::string timepointString)
    {
        std::chrono::sys_time<std::chrono::milliseconds> tp;

        std::istringstream stream(timepointString);

        stream >> std::chrono::parse("%FT%TZ", tp);

        if (stream.fail()) {
            throw std::runtime_error("Invalid timestamp: " + timepointString);
        }

        return tp;
    }
}
