//
// Created by Moritz Herzog on 26.08.24.
//

#include "MarkdownParser.h"


#include "CommonmarkParser/cmark.h"

#include <SysMLv2Standard/entities/Element.h>
#include <QFile>
#include <sstream>
#include <iostream>     // For the std::cin and std::cout. I don't like, but... oh well
#include <regex>        // Unlocks REGEX POWAA


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

    std::vector<SysMLv2::Entities::Element *> MarkdownParser::getElementsOfProject() {
        std::vector<SysMLv2::Entities::Element *> returnValue;
        auto child = cmark_node_first_child(MarkdownDocument);
        std::cout<<"Document Node Type: "<<cmark_node_get_type_string(MarkdownDocument)<<std::endl;
        std::cout<<"First Child Node Type: "<<cmark_node_get_type_string(child)<<std::endl;
        std::cout<< cmark_render_commonmark(child,0,0) <<std::endl;
        do {
            child = cmark_node_next(child);
            switch (cmark_node_get_type(child)) {
                case CMARK_NODE_NONE:
                    break;
                case CMARK_NODE_DOCUMENT:
                    break;
                case CMARK_NODE_BLOCK_QUOTE:
                    break;
                case CMARK_NODE_LIST:
                    break;
                case CMARK_NODE_ITEM:
                    break;
                case CMARK_NODE_CODE_BLOCK:
//                    SysMLv2::Entities::Element* element = new SysMLv2::Entities::Element();
                    break;
                case CMARK_NODE_HTML_BLOCK:
                    break;
                case CMARK_NODE_CUSTOM_BLOCK:
                    break;
                case CMARK_NODE_PARAGRAPH:
                    break;
                case CMARK_NODE_HEADING:
                    break;
                case CMARK_NODE_THEMATIC_BREAK:
                    break;
                case CMARK_NODE_TEXT:
                    break;
                case CMARK_NODE_SOFTBREAK:
                    break;
                case CMARK_NODE_LINEBREAK:
                    break;
                case CMARK_NODE_CODE:
                    break;
                case CMARK_NODE_HTML_INLINE:
                    break;
                case CMARK_NODE_CUSTOM_INLINE:
                    break;
                case CMARK_NODE_EMPH:
                    break;
                case CMARK_NODE_STRONG:
                    break;
                case CMARK_NODE_LINK:
                    break;
                case CMARK_NODE_IMAGE:
                    break;
            }
        }while(cmark_node_last_child(MarkdownDocument)!=child);
        return returnValue;
    }

    void MarkdownParser::parseInternally() {
        MarkdownDocument = cmark_parse_document(MarkdownString.c_str(),MarkdownString.size(),0);
        auto child = cmark_node_first_child(MarkdownDocument);
        std::cout<<"Document Node Type: "<<cmark_node_get_type_string(MarkdownDocument)<<std::endl;
        std::cout<<"First Child Node Type: "<<cmark_node_get_type_string(child)<<std::endl;
        std::cout<< cmark_render_commonmark(child,0,0) <<std::endl;
        do {
            child = cmark_node_next(child);
            std::cout<<"Child Node Type: "<<cmark_node_get_type_string(child)<<std::endl;
        }while(cmark_node_last_child(MarkdownDocument)!=child);
    }
}