//
// Created by Moritz Herzog on 06.01.25.
//

#include "YaMlMarkdown.h"
#include <BaseFuctions/StringExtention.hpp>
#include <vector>
#include <map>
namespace DigitalTwin {
    namespace Client {
        YaMLMarkdown::YaMLMarkdown(std::string input) : TagLength(3){
            MarkdownCode = input;
            updateVariables();
        }

        void YaMLMarkdown::updateVariables() {
            std::vector<std::string> keywords = {"usage", "name", "description", "title", "maintainer"};
            const auto splittedString = CPSBASELIB::STD_EXTENTION::StringExtention::splitString(MarkdownCode, '\n');
            std::map<std::string, std::string> mappingToElements;
            for(const auto& key : keywords ){
                for(const auto& elem : splittedString) {
                    if(elem.find(key)!=std::string::npos) {
                        mappingToElements[key] = CPSBASELIB::STD_EXTENTION::StringExtention::splitString(elem,':').back();
                    }
                }
            }

            if(mappingToElements.contains("usage"))
                Usage = mappingToElements["usage"];

            if(mappingToElements.contains("name"))
                ProjectName = mappingToElements["name"];

            if(mappingToElements.contains("description"))
                ProjectDescription = mappingToElements["description"];

            if(mappingToElements.contains("title"))
                ProjectTitle = mappingToElements["title"];

            if(mappingToElements.contains("maintainer"))
                Maintainer = mappingToElements["maintainer"];
        }

        std::vector<std::string> YaMLMarkdown::getRegExForElement() {
            std::vector<std::string> regexes = {"---[a-z]*\\n[\\s\\S]*?\\n---"};
            return regexes;
        }

        std::string YaMLMarkdown::getMarkdownOfElement() {
            return MarkdownCode;
        }

        std::string YaMLMarkdown::getHTMLStringOfElement() {
            std::string HtmlString = "<header> \r\n";
            HtmlString += "<h1>" + ProjectTitle + "</h1>\r\n";
            HtmlString += "<h2> Maintained by: " + Maintainer + "</h2>";
            return HtmlString;
        }
    } // Client
} // DigitalTwin