//
// Created by Moritz Herzog on 26.08.24.
//

#include "MarkdownParser.h"


#include "CommonmarkParser/cmark.h"

#include <kerml/root/elements/Element.h>
#include <kerml/root/annotations/TextualRepresentation.h>

#include <QFile>
#include <regex>
#include <cstdlib>


namespace DigitalTwin::Client {
    void MarkdownParser::parseMarkdownFile(QString path) {
        QFile file = QFile(path);
        [[maybe_unused]] auto returnValue = file.open(QIODevice::ReadOnly);
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

    std::vector<std::shared_ptr<KerML::Entities::Element>> MarkdownParser::getElementsOfProject() {
        std::vector<std::shared_ptr<KerML::Entities::Element>> returnValue;
        if (MarkdownDocument == nullptr)
            return returnValue;

        // One element per top level block of the document, in document order.
        for (auto child = cmark_node_first_child(MarkdownDocument); child != nullptr; child = cmark_node_next(child)) {
            switch (cmark_node_get_type(child)) {
                case CMARK_NODE_NONE:
                    break;
                case CMARK_NODE_HTML_BLOCK: {
                    // Raw HTML is document content, not model code: its own language keeps it out of the model.
                    const char* literal = cmark_node_get_literal(child);
                    returnValue.push_back(createElement("HTML", literal ? literal : ""));
                    break;
                }
                case CMARK_NODE_CODE_BLOCK:
                case CMARK_NODE_CODE: {
                    const char* fenceInfo = cmark_node_get_fence_info(child);
                    const char* literal = cmark_node_get_literal(child);
                    returnValue.push_back(createElement(fenceInfo ? fenceInfo : "", literal ? literal : ""));
                    break;
                }
                default: {
                    char* rendered = cmark_render_commonmark(child, 0, 0);
                    returnValue.push_back(createElement("Markdown", rendered ? rendered : ""));
                    free(rendered);
                    break;
                }
            }
        }
        return returnValue;
    }

    void MarkdownParser::parseInternally() {
        MarkdownDocument = cmark_parse_document(MarkdownString.c_str(),MarkdownString.size(),0);
    }

    std::shared_ptr<KerML::Entities::Element> MarkdownParser::createElement(std::string language, std::string body) {
        const auto element = std::make_shared<KerML::Entities::TextualRepresentation>();
        element->setLanguage(language);
        element->setBody(body);
        return element;
    }
}