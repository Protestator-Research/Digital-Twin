//
// Created by Moritz Herzog on 16.01.25.
//

#ifndef DIGITALTWIN_DIGITALTWINADDRESSEXCEPTION_H
#define DIGITALTWIN_DIGITALTWINADDRESSEXCEPTION_H

#include <exception>

namespace DigitalTwin::Model {
    class DigitalTwinAddressException : public std::exception {
    public:
        DigitalTwinAddressException();
        ~DigitalTwinAddressException() override;

        const char * what() const noexcept override;

    };
}


#endif //DIGITALTWIN_DIGITALTWINADDRESSEXCEPTION_H
