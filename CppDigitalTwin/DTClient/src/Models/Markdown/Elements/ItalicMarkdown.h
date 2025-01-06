//
// Created by Moritz Herzog on 06.01.25.
//

#ifndef DIGITALTWIN_ITALICMARKDOWN_H
#define DIGITALTWIN_ITALICMARKDOWN_H

#include "IMarkdownElement.h"

#include <vector>

namespace DigitalTwin::Client {
    class ItalicMarkdown : public IMarkdownElement {
    public:
        ItalicMarkdown() = delete;

        explicit ItalicMarkdown(std::string input);

        virtual ~ItalicMarkdown() = default;

        std::string getHTMLStringOfElement() override;

        std::string getMarkdownOfElement() override;

        static std::vector<std::string> getRegExForElement();
    private:
        void setContentString();

        const uint8_t TagLength;

        std::string MarkdownContent;
        std::string Content;
    };
}

#endif //DIGITALTWIN_ITALICMARKDOWN_H
