//
// Created by Moritz Herzog on 06.01.25.
//

#ifndef DIGITALTWIN_HEADERSMARKDOWN_H
#define DIGITALTWIN_HEADERSMARKDOWN_H

#include <vector>

#include "IMarkdownElement.h"

namespace DigitalTwin {
    namespace Client {

        class HeadersMarkdown : public IMarkdownElement {
            enum HeadingLevel {
                H1 = 1,
                H2,
                H3,
                H4,
                H5,
                H6
            };
        public:
            HeadersMarkdown() = delete;
            explicit HeadersMarkdown(std::string input);
            virtual ~HeadersMarkdown() = default;

            std::string getHTMLStringOfElement() override;
            std::string getMarkdownOfElement() override;

            static std::vector<std::string> getRegExForElement();
        private:
            void setHeadingTextAndLevel();

            std::string MarkdownText;
            std::string HeadingText;
            HeadingLevel Level;
        };

    } // Client
} // DigitalTwin

#endif //DIGITALTWIN_HEADERSMARKDOWN_H
