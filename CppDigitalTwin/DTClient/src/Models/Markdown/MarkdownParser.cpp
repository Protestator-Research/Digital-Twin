//
// Created by Moritz Herzog on 26.08.24.
//

#include "MarkdownParser.h"


#include "CommonmarkParser/cmark.h"

#include <sysmlv2/rest/entities/Element.h>
#include <QFile>
#include <regex>


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
        return QString(cmark_render_html(MarkdownDocument,0));
    }

    QString MarkdownParser::getMarkdownString()
    {
        return QString::fromStdString(MarkdownString);
    }

    std::vector<std::shared_ptr<SysMLv2::REST::Element>> MarkdownParser::getElementsOfProject() {
        std::vector<std::shared_ptr<SysMLv2::REST::Element>> returnValue;
        auto child = cmark_node_first_child(MarkdownDocument);
        switch (cmark_node_get_type(child)) {
            case CMARK_NODE_NONE:
                break;
            case CMARK_NODE_HTML_BLOCK:
            case CMARK_NODE_CODE_BLOCK:
            case CMARK_NODE_CODE:
                returnValue.push_back(createElement(cmark_node_get_fence_info(child),cmark_node_get_literal(child)));
                break;
            case CMARK_NODE_CUSTOM_BLOCK:
            case CMARK_NODE_PARAGRAPH:
            case CMARK_NODE_HEADING:
            case CMARK_NODE_THEMATIC_BREAK:
            case CMARK_NODE_TEXT:
            case CMARK_NODE_SOFTBREAK:
            case CMARK_NODE_LINEBREAK:
            case CMARK_NODE_HTML_INLINE:
            case CMARK_NODE_CUSTOM_INLINE:
            case CMARK_NODE_EMPH:
            case CMARK_NODE_STRONG:
            case CMARK_NODE_LINK:
            case CMARK_NODE_IMAGE:
            case CMARK_NODE_DOCUMENT:
            case CMARK_NODE_BLOCK_QUOTE:
            case CMARK_NODE_LIST:
            case CMARK_NODE_ITEM:
                returnValue.push_back(createElement("Markdown", cmark_render_commonmark(child,0,0)));
                break;
        }
        do {
            child = cmark_node_next(child);
            switch (cmark_node_get_type(child)) {
                case CMARK_NODE_NONE:
                    break;
                case CMARK_NODE_HTML_BLOCK:
                case CMARK_NODE_CODE_BLOCK:
                case CMARK_NODE_CODE:
                    returnValue.push_back(createElement(cmark_node_get_fence_info(child),cmark_node_get_literal(child)));
                    break;
                case CMARK_NODE_CUSTOM_BLOCK:
                case CMARK_NODE_PARAGRAPH:
                case CMARK_NODE_HEADING:
                case CMARK_NODE_THEMATIC_BREAK:
                case CMARK_NODE_TEXT:
                case CMARK_NODE_SOFTBREAK:
                case CMARK_NODE_LINEBREAK:
                case CMARK_NODE_HTML_INLINE:
                case CMARK_NODE_CUSTOM_INLINE:
                case CMARK_NODE_EMPH:
                case CMARK_NODE_STRONG:
                case CMARK_NODE_LINK:
                case CMARK_NODE_IMAGE:
                case CMARK_NODE_DOCUMENT:
                case CMARK_NODE_BLOCK_QUOTE:
                case CMARK_NODE_LIST:
                case CMARK_NODE_ITEM:
                    returnValue.push_back(createElement("Markdown", cmark_render_commonmark(child,0,0)));
                    break;
            }
        }while(cmark_node_last_child(MarkdownDocument)!=child);
        return returnValue;
    }

    void MarkdownParser::parseInternally() {
        MarkdownDocument = cmark_parse_document(MarkdownString.c_str(),MarkdownString.size(),0);
    }

    std::shared_ptr<SysMLv2::REST::Element> MarkdownParser::createElement(std::string language, std::string body) {
        std::shared_ptr<SysMLv2::REST::Element> element = std::make_shared<SysMLv2::REST::Element>();
        element->setLanguage(language);
        element->setBody(body);
        return element;
    }
}