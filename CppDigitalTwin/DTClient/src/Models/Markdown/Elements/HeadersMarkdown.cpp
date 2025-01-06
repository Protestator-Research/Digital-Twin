//
// Created by Moritz Herzog on 06.01.25.
//

#include "HeadersMarkdown.h"
#include <vector>
#include <regex>

namespace DigitalTwin {
    namespace Client {

        HeadersMarkdown::HeadersMarkdown(std::string input) {
            MarkdownText = input;
            setHeadingTextAndLevel();
        }

        std::string HeadersMarkdown::getHTMLStringOfElement() {
            return "<h"+std::to_string((int)Level)+">" + HeadingText + "</h"+std::to_string((int)Level)+"><hr /><br />";
        }

        std::string HeadersMarkdown::getMarkdownOfElement() {
            return MarkdownText;
        }

        std::vector<std::string> HeadersMarkdown::getRegExForElement() {
            std::vector<std::string> regex = {"#+[^\\r\\n]+(\\r|\\n|\\r\\n)"};
            return regex;
        }

        void HeadersMarkdown::setHeadingTextAndLevel() {
            std::regex headers(getRegExForElement()[0]); // Regex for the headers
            std::smatch match;  // Match object (stores match information)
            HeadingText = MarkdownText;
            while (std::regex_search (MarkdownText, match, headers)) {

                int match_begin = match.position();
                //int match_end   = match_begin + match.length();

                //printf("Processing match: %s", HTMLString.substr(match_begin, match.length()).c_str());

                int j;
                for(j = 0; MarkdownText[j+match_begin]=='#'; j++){}     // So count the number of '#' in a row,
                Level = (HeadingLevel) j;

                HeadingText.replace(match_begin, j, "");
            }
        }
    } // Client
} // DigitalTwin