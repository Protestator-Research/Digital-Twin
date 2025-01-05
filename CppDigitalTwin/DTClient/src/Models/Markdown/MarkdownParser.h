//
// Created by Moritz Herzog on 26.08.24.
//

#ifndef DIGITALTWIN_MARKDOWNPARSER_H
#define DIGITALTWIN_MARKDOWNPARSER_H

#include <QString>

#include <vector>

namespace SysMLv2::Entities {
    class Element;
}

namespace DigitalTwin::Client {
    class MarkdownParser {
    public:
        struct token {
            std::string rex;    // Regex to find the markdown token
            int length1;        // Length of the first token ("**" = 2, "*" = 1)
            int length2;        // Length of the last token (quotes only have at the beginning, newlines at the end, etc)
            std::string tagOpen, tagClose;    // Equivalent tag
        };

        MarkdownParser() = default;


        void parseMarkdownFile(QString path);
        void parseMarkdown(QString markdown);

        std::vector<SysMLv2::Entities::Element*> getElementsOfProject();
        QString getHTMLOfMarkdown();
        QString getMarkdownString();

    private:
        std::string MarkdownString;
        std::string HTMLString;
        void parseInternally();
    };
}

#endif //DIGITALTWIN_MARKDOWNPARSER_H
