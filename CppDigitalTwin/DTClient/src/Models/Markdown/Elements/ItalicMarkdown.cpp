//
// Created by Moritz Herzog on 06.01.25.
//

#include "ItalicMarkdown.h"
#include <vector>
#include <regex>

namespace DigitalTwin::Client {
    ItalicMarkdown::ItalicMarkdown(std::string input) :
    TagLength(1){
        MarkdownContent = input;
        Content = MarkdownContent;
        setContentString();
    }

    std::string ItalicMarkdown::getHTMLStringOfElement() {
        return "<i>" + Content + "</i>";
    }

    std::string ItalicMarkdown::getMarkdownOfElement() {
        return MarkdownContent;
    }

    std::vector<std::string> ItalicMarkdown::getRegExForElement() {
        std::vector<std::string> regexes = {"\\*[^\\*\\r\\n]+\\*", "_[^\\*\\r\\n]+_"};
        return regexes;
    }

    void ItalicMarkdown::setContentString() {
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
}