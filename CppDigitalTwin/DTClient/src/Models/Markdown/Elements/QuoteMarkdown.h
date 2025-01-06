//
// Created by Moritz Herzog on 06.01.25.
//

#ifndef DIGITALTWIN_QUOTEMARKDOWN_H
#define DIGITALTWIN_QUOTEMARKDOWN_H

#include "IMarkdownElement.h"
#include <string>
#include <vector>

namespace DigitalTwin {
    namespace Client {

        class QuoteMarkdown  : public IMarkdownElement {
        public:
            QuoteMarkdown() = delete;
            explicit QuoteMarkdown(std::string input);
            virtual ~QuoteMarkdown() = default;

            std::string getHTMLStringOfElement() override;
            std::string getMarkdownOfElement() override;

            static std::vector<std::string> getRegExForElement();
        private:
            std::string QuotedText;
        };

    } // Client
} // DigitalTwin

#endif //DIGITALTWIN_QUOTEMARKDOWN_H
