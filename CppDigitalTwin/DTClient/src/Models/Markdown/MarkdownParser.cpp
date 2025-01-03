//
// Created by Moritz Herzog on 26.08.24.
//

#include "MarkdownParser.h"

#include <md4c.h>
#include <QFile>

namespace DigitalTwin::Client {
    void MarkdownParser::parseMarkdown(QString path) {
        QFile file = QFile(path);
        file.open(QIODevice::ReadOnly);
        QByteArray array = file.readAll();
        file.close();
        char* markdown = array.data();
        MD_PARSER* parser = new MD_PARSER();
        int result = md_parse(markdown, sizeof(markdown),parser,NULL);

    }
}