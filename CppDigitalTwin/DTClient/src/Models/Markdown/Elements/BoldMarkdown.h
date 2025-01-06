//
// Created by Moritz Herzog on 06.01.25.
//

#ifndef DIGITALTWIN_BOLDMARKDOWN_H
#define DIGITALTWIN_BOLDMARKDOWN_H

#include "IMarkdownElement.h"

#include <vector>

namespace DigitalTwin {
    namespace Client {

        class BoldMarkdown : public IMarkdownElement {
        public:
            BoldMarkdown() = delete;

            explicit BoldMarkdown(std::string input);

            virtual ~BoldMarkdown() = default;

            std::string getHTMLStringOfElement() override;

            std::string getMarkdownOfElement() override;

            static std::vector<std::string> getRegExForElement();

        private:
            void setContentString();

            const uint8_t TagLength;

            std::string MarkdownContent;
            std::string Content;
        };

    } // Client
} // DigitalTwin

#endif //DIGITALTWIN_BOLDMARKDOWN_H
