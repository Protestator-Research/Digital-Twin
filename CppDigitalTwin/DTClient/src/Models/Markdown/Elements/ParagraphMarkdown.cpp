//
// Created by Moritz Herzog on 06.01.25.
//

#include "ParagraphMarkdown.h"

#include <vector>

namespace DigitalTwin {
    namespace Client {
        ParagraphMarkdown::ParagraphMarkdown(std::string input) {
            ParagraphMarkdownText = input;
        }

        std::string ParagraphMarkdown::getHTMLStringOfElement() {
            return "<p>" + ParagraphMarkdownText + "</p>";
        }

        std::string ParagraphMarkdown::getMarkdownOfElement() {
            return ParagraphMarkdownText;
        }

        std::vector<std::string> ParagraphMarkdown::getRegExForElement() {
            return std::vector<std::string>();
        }
    } // Client
} // DigitalTwin