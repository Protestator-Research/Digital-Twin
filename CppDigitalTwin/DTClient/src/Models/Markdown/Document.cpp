//
// Created by Moritz Herzog on 03.01.25.
//

#include "Document.h"
namespace DigitalTwin::Model {
    Document::Document(QObject *parent) : QObject(parent) {

    }

    void Document::setText(const QString &text) {
        if(text == Text)
            return;
        Text = text;
        emit textChanged(Text);
    }

}
