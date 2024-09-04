//
// Created by Moritz Herzog on 26.08.24.
//

#ifndef DIGITALTWIN_MARKDOWNPARSER_H
#define DIGITALTWIN_MARKDOWNPARSER_H

#include <QString>

namespace DigitalTwin::Client {
    class MarkdownParser {
    public:
        MarkdownParser() = delete;

        static void parseMarkdown(QString path);

    private:
    };
}

#endif //DIGITALTWIN_MARKDOWNPARSER_H
