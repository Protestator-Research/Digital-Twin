//
// Created by Moritz Herzog on 06.01.25.
//

#ifndef DIGITALTWIN_CODEMARKDOWN_H
#define DIGITALTWIN_CODEMARKDOWN_H

#include "IMarkdownElement.h"

#include <vector>

namespace DigitalTwin {
    namespace Client {

        class CodeMarkdown : public IMarkdownElement{
        public:
            CodeMarkdown() = delete;
            explicit CodeMarkdown(std::string input);
            virtual ~CodeMarkdown() = default;

            std::string getHTMLStringOfElement() override;
            std::string getMarkdownOfElement() override;

            static std::vector<std::string> getRegExForElement();

        private:
            void fillCodeAndLanguage();
            const uint8_t TagLength;
            std::string MarkdownContent;
            std::string Language;
            std::string Code;
        };

    } // Client
} // DigitalTwin

#endif //DIGITALTWIN_CODEMARKDOWN_H
