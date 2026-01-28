//
// Created by Moritz Herzog on 25.09.24.
//

#ifndef DIGITALTWIN_PORT_H
#define DIGITALTWIN_PORT_H

#include "IDigitalTwinElement.h"

#include <string>
#include "../../cpp_digital_twin_lib_global.h"

namespace DigitalTwin::Model {
    class CPPDIGITALTWINLIB_EXPORT Port : public IDigitalTwinElement {
    public:
        Port() = delete;
        explicit Port(std::string name);

        ~Port() = default;

    };
}

#endif //DIGITALTWIN_PORT_H
