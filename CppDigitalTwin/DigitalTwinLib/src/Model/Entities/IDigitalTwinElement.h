//
// Created by Moritz Herzog on 11.09.24.
//

#ifndef DIGITALTWIN_IDIGITALTWINELEMENT_H
#define DIGITALTWIN_IDIGITALTWINELEMENT_H

#include <string>

namespace DigitalTwin::Model {
    class IDigitalTwinElement {
    public:
        IDigitalTwinElement() = delete;

        IDigitalTwinElement(std::string name){
            Name = name;
        }

        virtual ~IDigitalTwinElement() = default;

        std::string getName() {
            return Name;
        };

    protected:
        std::string Name;
    };
}

#endif //DIGITALTWIN_IDIGITALTWINELEMENT_H
