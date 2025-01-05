//
// Created by Moritz Herzog on 26.08.24.
//

#include "MarkdownParser.h"

#include <QFile>
#include <sstream>
#include <string>       // Use strings
#include <fstream>      // Create, write and read files
#include <iostream>     // For the std::cin and std::cout. I don't like, but... oh well
#include <regex>        // Unlocks REGEX POWAA
#include <algorithm>    // It does one linear scan of the string and inplace replaces all the matching characters
#include <utility>

namespace DigitalTwin::Client {
    void MarkdownParser::parseMarkdownFile(QString path) {
        QFile file = QFile(path);
        file.open(QIODevice::ReadOnly);
        QByteArray array = file.readAll();
        file.close();
        MarkdownString = QString(array.data()).toStdString();
        parseInternally();
    }

    void MarkdownParser::parseMarkdown(QString markdown)
    {
        MarkdownString = markdown.toStdString();
        parseInternally();
    }

    QString MarkdownParser::getHTMLOfMarkdown() {
        return QString::fromStdString(HTMLString);
    }

    QString MarkdownParser::getMarkdownString()
    {
        return QString::fromStdString(MarkdownString);
    }

    std::vector<SysMLv2::Entities::Element *> MarkdownParser::getElementsOfProject() {
        return std::vector<SysMLv2::Entities::Element *>();
    }

    void MarkdownParser::parseInternally() {
        std::smatch match;  // Match object (stores match information)

        HTMLString = MarkdownString;

        ////////////////////////////// HEADERS //////////////////////////////////////
        std::regex headers("#+[^\\r\\n]+(\\r|\\n|\\r\\n)"); // Regex for the headers

        while (std::regex_search (HTMLString, match, headers)) {

            int match_begin = match.position();
            int match_end   = match_begin + match.length();

            printf("Processing match: %s", HTMLString.substr(match_begin, match.length()).c_str());

            int j;
            for(j = 0; HTMLString[j+match_begin]=='#'; j++){}     // So count the number of '#' in a row,


            HTMLString.insert(match_end, "</h" + std::to_string(j) + "><hr/>");
            HTMLString.replace(match_begin, j, "<h" + std::to_string(j) + ">");
        }

        //////////////////////////////// LINKS //////////////////////////////////////
        std::regex links("\\[[^\\n\\r]*\\]\\([^\\n\\r]*\\)"); // Regex for the headers
        std::regex linkHref("\\([^\\n\\r]*\\)");

        std::smatch match2;  // Match object (stores match information)
        while (std::regex_search (HTMLString, match, links)) {

            int match_begin = match.position();
            //int match_end   = match_begin + match.length();

            std::string text = "";
            std::string link = "";

            int i, count;
            for(i = match_begin+1, count = 1; count > 0; i++){
                char c = HTMLString[i];

                if(c == '[') count++;
                else if(c == ']') count--;

                if(count > 0) text += c;
            }

            for(i += 1, count = 1; count > 0; i++){
                char c = HTMLString[i];

                if(c == '(') count++;
                else if(c == ')') count--;

                if(count > 0) link += c;
            }

            std::string finalLink = "<a href=\"" + link + "\">" + text + "</a>";

            //printf("Processing link: %s", finalLink.c_str());
            HTMLString.replace(match_begin, match.length(), finalLink);
        }

        ////////////////////////// OTHER TOKENS ////////////////////////////////////
        //token{"[^\\r\\n]+((\\r|\\n|\\r\\n)[^\\r\\n]+)*",0,2,  "p"}       // Paragraphs regex

        std::list<token> tokens = {
                token{"`[^`\\n]+`",                             1,1,  "<code>",         "</code>"},         // Code tag regex
                token{"(\\*){2}[^(\\*){2}\\r\\n]+(\\*){2}",     2,2,  "<b>",            "</b>"},            // Bold tag regex    (**)
                token{"(_){2}[^(_){2}\\r\\n]+(_){2}",           2,2,  "<b>",            "</b>"},            // Bold tag regex    (__)
                token{"\\*[^\\*\\r\\n]+\\*",                    1,1,  "<i>",            "</i>"},            // Italic tag regex  (*)
                token{"_[^\\*\\r\\n]+_",                        1,1,  "<i>",            "</i>"},            // Italic tag regex  (_)
                token{"```[a-z]*\\n[\\s\\S]*?\\n```",           3,3,  "<pre><code>",    "</code></pre>"}    // Multiline code block
                //token{"(^>.*?\n{2})",                           1,0,  "<quote>",        "</quote>"}         // Quote tag regex
        };

        for(token tok: tokens){
            //printf("Checking for %s\n", tok.tagOpen.c_str());
            std::regex tmp(tok.rex);

            int match_end, match_begin;
            while (std::regex_search (HTMLString, match, tmp)) {
                match_begin = match.position();
                match_end   = match_begin + match.length();

                printf("Processing match: %s\n", HTMLString.substr(match_begin, match.length()).c_str());

                HTMLString.replace(match_end-tok.length2, tok.length2, tok.tagClose);
                HTMLString.replace(match_begin,           tok.length1, tok.tagOpen);
            }

            //printf("Done checking %s\n", tok.tagOpen.c_str());
        }

        ////////////////////////////// LINE BREAKS ///////////////////////////////////
        for(size_t i = 0; i<HTMLString.length(); i++){
            if(HTMLString[i] == '\n'){
                HTMLString.insert(i, "<br/>");
                i+=6;
            }
        }
    }
}