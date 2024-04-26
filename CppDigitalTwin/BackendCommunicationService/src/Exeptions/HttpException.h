//
// Created by Moritz Herzog on 24.04.24.
//

#ifndef DIGITALTWIN_HTTPEXCEPTION_H
#define DIGITALTWIN_HTTPEXCEPTION_H

#include <exception>

namespace BACKEND_COMMUNICATION::EXCEPTIONS {
    class HTTPException : std::exception {
    public:
        HTTPException() = delete;
        HTTPException(long httpErrorCode);

    private:
        [[maybe_unused]] long HttpErrorCode;
    };
}

#endif //DIGITALTWIN_HTTPEXCEPTION_H
