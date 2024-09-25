//
// Created by Moritz Herzog on 25.09.24.
//

#ifndef DIGITALTWIN_PORT_H
#define DIGITALTWIN_PORT_H

#include "IDigitalTwinElement.h"

#include <string>

namespace DigitalTwin::Model {
    class Port : public IDigitalTwinElement {
    public:
        Port() = delete;
        explicit Port(std::string name);

        ~Port() = default;

    };
}

#endif //DIGITALTWIN_PORT_H
