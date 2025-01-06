//
// Created by Moritz Herzog on 06.01.25.
//

#include "BoldMarkdown.h"
#include <vector>
#include <regex>

namespace DigitalTwin {
    namespace Client {
        BoldMarkdown::BoldMarkdown(std::string input) :
        TagLength (2){
            MarkdownContent = input;
            Content = MarkdownContent;
            setContentString();
        }

        std::vector<std::string> BoldMarkdown::getRegExForElement() {
            std::vector<std::string> regexes = {"(\\*){2}[^(\\*){2}\\r\\n]+(\\*){2}", "(_){2}[^(_){2}\\r\\n]+(_){2}"};
            return regexes;
        }

        std::string BoldMarkdown::getHTMLStringOfElement() {
            return "<b>" + Content + "</b>";
        }

        std::string BoldMarkdown::getMarkdownOfElement() {
            return MarkdownContent;
        }

        void BoldMarkdown::setContentString() {
            std::smatch match;
            for(const auto &rex : getRegExForElement()) {
                std::regex tmp(rex);

                int match_end, match_begin;
                while (std::regex_search(Content, match, tmp)) {
                    match_begin = match.position();
                    match_end = match_begin + match.length();

                    //printf("Processing match: %s\n", HTMLString.substr(match_begin, match.length()).c_str());

                    Content.replace(match_end - TagLength, TagLength, "");
                    Content.replace(match_begin, TagLength, "");
                }
            }
        }

    } // Client
} // DigitalTwin