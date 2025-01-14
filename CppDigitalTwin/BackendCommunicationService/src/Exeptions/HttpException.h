//
// Created by Moritz Herzog on 24.04.24.
//

#ifndef DIGITALTWIN_HTTPEXCEPTION_H
#define DIGITALTWIN_HTTPEXCEPTION_H

#include <exception>
#include "../cpp_backend_communication_global.h"

namespace BACKEND_COMMUNICATION::EXCEPTIONS {
    class CPSBACKENDCOMMUNICATION_EXPORT HTTPException : std::exception {
    public:
        HTTPException() = delete;
        HTTPException(long httpErrorCode);

        const char * what() const noexcept override;
    private:
        [[maybe_unused]] long HttpErrorCode;
    };
}

#endif //DIGITALTWIN_HTTPEXCEPTION_H
