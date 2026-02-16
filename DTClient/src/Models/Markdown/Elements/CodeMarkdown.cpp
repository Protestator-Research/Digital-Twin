//
// Created by Moritz Herzog on 06.01.25.
//

#include "CodeMarkdown.h"
#include <vector>
#include <regex>
#include <BaseFuctions/StringExtention.hpp>

namespace DigitalTwin {
    namespace Client {
        CodeMarkdown::CodeMarkdown(std::string input) : TagLength(3)
        {
            MarkdownContent = input;
            Code=MarkdownContent;
            fillCodeAndLanguage();
        }

        std::string CodeMarkdown::getHTMLStringOfElement() {
            return "<pre><code style=\"font-family: \"Courier New\";\">" + Language + "</code></pre>";
        }

        std::string CodeMarkdown::getMarkdownOfElement() {
            return MarkdownContent;
        }

        std::vector<std::string> CodeMarkdown::getRegExForElement() {
            std::vector<std::string> regExes = {"```[a-z]*\\n[\\s\\S]*?\\n```", "~~~[a-z]*\\n[\\s\\S]*?\\n~~~"};
            return regExes;
        }

        void CodeMarkdown::fillCodeAndLanguage() {
            std::smatch match;
            for(const auto &rex : getRegExForElement()) {
                std::regex tmp(rex);

                int match_end, match_begin;
                while (std::regex_search(Code, match, tmp)) {
                    match_begin = match.position();
                    match_end = match_begin + match.length();

                    //printf("Processing match: %s\n", HTMLString.substr(match_begin, match.length()).c_str());

                    Code.replace(match_end - TagLength, TagLength, "");
                    Code.replace(match_begin, TagLength, "");
                }
            }
            const auto elements = CPSBASELIB::STD_EXTENTION::StringExtention::splitString(Code,'\n');
            Language = elements[0];
            std::string realCode = "";
            for(size_t i = 1; i<elements.size(); i++) {
                realCode += elements[i];
            }
            Code = realCode;
        }
    } // Client
} // DigitalTwin