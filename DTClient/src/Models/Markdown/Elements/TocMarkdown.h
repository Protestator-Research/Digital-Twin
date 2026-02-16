//
// Created by Moritz Herzog on 06.01.25.
//

#ifndef DIGITALTWIN_TOCMARKDOWN_H
#define DIGITALTWIN_TOCMARKDOWN_H

#include "IMarkdownElement.h"
#include <memory>
#include <vector>

namespace DigitalTwin {
    namespace Client {

        class TocMarkdown : public IMarkdownElement{
        public:
            TocMarkdown() = delete;
            explicit TocMarkdown(std::string input);
            virtual ~TocMarkdown();

            std::string getHTMLStringOfElement() override;
            std::string getMarkdownOfElement() override;

            static std::vector<std::string> getRegExForElement();
        private:
            std::vector<std::shared_ptr<IMarkdownElement>> Headings;
        };

    } // Client
} // DigitalTwin

#endif //DIGITALTWIN_TOCMARKDOWN_H
