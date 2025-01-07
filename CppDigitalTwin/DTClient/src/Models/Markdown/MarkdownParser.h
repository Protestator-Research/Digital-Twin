//
// Created by Moritz Herzog on 26.08.24.
//

#ifndef DIGITALTWIN_MARKDOWNPARSER_H
#define DIGITALTWIN_MARKDOWNPARSER_H

#include "Elements/IMarkdownElement.h"
#include "CommonmarkParser/cmark.h"

#include <QString>
#include <vector>
#include <memory>
#include <map>
#include <functional>

namespace SysMLv2::Entities {
    class Element;
}

namespace DigitalTwin::Client {
    class MarkdownParser {
    public:

        MarkdownParser() = default;


        void parseMarkdownFile(QString path);
        void parseMarkdown(QString markdown);

        std::vector<SysMLv2::Entities::Element*> getElementsOfProject();
        QString getHTMLOfMarkdown();
        QString getMarkdownString();

    private:
        std::string MarkdownString;
        cmark_node* MarkdownDocument;

        SysMLv2::Entities::Element* createElement(std::string language, std::string body);
        void parseInternally();
    };
}

#endif //DIGITALTWIN_MARKDOWNPARSER_H
