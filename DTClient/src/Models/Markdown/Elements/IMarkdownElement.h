//
// Created by Moritz Herzog on 06.01.25.
//

#ifndef DIGITALTWIN_IMARKDOWNELEMENT_H
#define DIGITALTWIN_IMARKDOWNELEMENT_H

#include <string>

namespace DigitalTwin::Client {
    class IMarkdownElement {
    public:
        virtual std::string getHTMLStringOfElement() = 0;
        virtual std::string getMarkdownOfElement() = 0;
    };
}

#endif //DIGITALTWIN_IMARKDOWNELEMENT_H
