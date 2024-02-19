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
}