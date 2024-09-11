//
// Created by Moritz Herzog on 05.01.24.
//

#pragma once

#include <stdexcept>
#include "../cpp_backend_communication_global.h"


namespace BACKEND_COMMUNICATION::EXCEPTIONS {
    class CPSBACKENDCOMMUNICATION_EXPORT NotEnoughMemoryError : public std::runtime_error {
    public:
        NotEnoughMemoryError();
        ~NotEnoughMemoryError();
    };
}
