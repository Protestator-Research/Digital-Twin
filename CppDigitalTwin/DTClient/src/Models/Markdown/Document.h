//
// Created by Moritz Herzog on 03.01.25.
//

#ifndef DIGITALTWIN_DOCUMENT_H
#define DIGITALTWIN_DOCUMENT_H

#include <QObject>

namespace DigitalTwin::Model {
    class Document : public QObject {
        Q_OBJECT
        Q_PROPERTY(QString text MEMBER m_text NOTIFY textChanged FINAL)
    public:
        explicit Document(QObject* parent = NULL);
        ~Document() = default;

        void setText(const QString& text);

    signals:
        void textChanged(const QString &text);

    private:
        QString Text;
    };
}


#endif //DIGITALTWIN_DOCUMENT_H
