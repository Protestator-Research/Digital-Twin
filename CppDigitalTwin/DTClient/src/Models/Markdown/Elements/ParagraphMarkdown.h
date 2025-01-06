//
// Created by Moritz Herzog on 06.01.25.
//

#ifndef DIGITALTWIN_PARAGRAPHMARKDOWN_H
#define DIGITALTWIN_PARAGRAPHMARKDOWN_H

#include "IMarkdownElement.h"

#include <vector>

namespace DigitalTwin {
    namespace Client {

        class ParagraphMarkdown : public IMarkdownElement {
        public:
            ParagraphMarkdown() = delete;

            explicit ParagraphMarkdown(std::string input);

            virtual ~ParagraphMarkdown() = default;

            std::string getHTMLStringOfElement() override;

            std::string getMarkdownOfElement() override;

            static std::vector<std::string> getRegExForElement();
        private:
            std::string ParagraphMarkdownText;
            std::string ParagraphHTMLText;

        };

    } // Client
} // DigitalTwin

#endif //DIGITALTWIN_PARAGRAPHMARKDOWN_H
