//
// Created by Moritz Herzog on 06.01.25.
//

#ifndef DIGITALTWIN_IMAGESMARKDOWN_H
#define DIGITALTWIN_IMAGESMARKDOWN_H

#include "IMarkdownElement.h"
#include <vector>

namespace DigitalTwin {
    namespace Client {

        class ImagesMarkdown : public IMarkdownElement {
        public:
            ImagesMarkdown() = delete;

            ImagesMarkdown(std::string input);

            virtual ~ImagesMarkdown() = default;

            std::string getHTMLStringOfElement() override;

            std::string getMarkdownOfElement() override;

            static std::vector<std::string> getRegExForElement();
        private:
            std::string PathToImage;
        };

    } // Client
} // DigitalTwin

#endif //DIGITALTWIN_IMAGESMARKDOWN_H
