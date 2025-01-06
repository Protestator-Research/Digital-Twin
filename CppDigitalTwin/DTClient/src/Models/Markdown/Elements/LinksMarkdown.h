//
// Created by Moritz Herzog on 06.01.25.
//

#ifndef DIGITALTWIN_LINKSMARKDOWN_H
#define DIGITALTWIN_LINKSMARKDOWN_H

#include <vector>

#include "IMarkdownElement.h"

namespace DigitalTwin {
    namespace Client {

        class LinksMarkdown : public IMarkdownElement {
        public:
            LinksMarkdown() = delete;

            explicit LinksMarkdown(std::string input);

            virtual ~LinksMarkdown() = default;

            std::string getHTMLStringOfElement() override;

            std::string getMarkdownOfElement() override;

            static std::vector<std::string> getRegExForElement();
        private:
            std::string URL;
            std::string LinkText;
        };

    } // Client
} // DigitalTwin

#endif //DIGITALTWIN_LINKSMARKDOWN_H
