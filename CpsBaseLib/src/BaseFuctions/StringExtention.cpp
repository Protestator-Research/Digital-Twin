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
        int y, mo, d, h, mi, s;
        int consumed = 0;
        if (std::sscanf(timepointString.c_str(), "%4d-%2d-%2dT%2d:%2d:%2d%n",
                        &y, &mo, &d, &h, &mi, &s, &consumed) != 6)
            throw std::runtime_error("Invalid timestamp: " + timepointString);

        // optionale Sekundenbruchteile (.123), auf Millisekunden normiert
        const char* p = timepointString.c_str() + consumed;
        std::chrono::milliseconds frac{0};
        if (*p == '.') {
            int digits = 0, value = 0;
            for (++p; *p >= '0' && *p <= '9'; ++p)
                if (digits < 3) { value = value * 10 + (*p - '0'); ++digits; }
            while (digits++ < 3) value *= 10;
            frac = std::chrono::milliseconds{value};
        }
        if (*p != 'Z' || *(p + 1) != '\0')
            throw std::runtime_error("Invalid timestamp: " + timepointString);

        std::chrono::year_month_day ymd{std::chrono::year{y}, std::chrono::month{static_cast<unsigned>(mo)}, std::chrono::day{static_cast<unsigned>(d)}};
        if (!ymd.ok())
            throw std::runtime_error("Invalid timestamp: " + timepointString);

        return std::chrono::sys_days{ymd} + std::chrono::hours{h} + std::chrono::minutes{mi} + std::chrono::seconds{s} + frac;
    }

    std::string StringExtention::replaceAll(std::string str, const std::string& from, const std::string& to)
    {
        if (from.empty()) return str;   // sonst Endlosschleife

        std::size_t pos = 0;
        while ((pos = str.find(from, pos)) != std::string::npos) {
            str.replace(pos, from.length(), to);
            pos += to.length();          // hinter die Ersetzung springen
        }
        return str;
    }
}
