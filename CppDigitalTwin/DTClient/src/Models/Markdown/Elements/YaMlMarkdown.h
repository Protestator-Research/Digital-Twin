//
// Created by Moritz Herzog on 06.01.25.
//

#ifndef DIGITALTWIN_YAMLMARKDOWN_H
#define DIGITALTWIN_YAMLMARKDOWN_H

#include <vector>

#include "IMarkdownElement.h"

namespace DigitalTwin {
    namespace Client {

        class YaMLMarkdown : public IMarkdownElement{
        public:
            YaMLMarkdown() = delete;
            explicit YaMLMarkdown(std::string input);
            virtual ~YaMLMarkdown() = default;

            std::string getHTMLStringOfElement() override;
            std::string getMarkdownOfElement() override;

            static std::vector<std::string> getRegExForElement();

        private:
            std::string MarkdownCode;
            const uint8_t TagLength;
            void updateVariables();

            std::string Usage;
            std::string ProjectName;
            std::string ProjectDescription;
            std::string ProjectTitle;
            std::string Maintainer;
        };

    } // Client
} // DigitalTwin

#endif //DIGITALTWIN_YAMLMARKDOWN_H
