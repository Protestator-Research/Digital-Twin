//
// Created by Moritz Herzog on 16.01.25.
//

#include "DigitalTwinAddressException.h"

namespace DigitalTwin::Model {
    DigitalTwinAddressException::~DigitalTwinAddressException() {

    }

    DigitalTwinAddressException::DigitalTwinAddressException() {

    }

    const char *DigitalTwinAddressException::what() const noexcept {
        return "Given Address was wrong!";
    }

}