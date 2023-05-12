//
// Created by moritzh on 12.05.23.
//

#ifndef DIGITWESTER_SERVERNOTREACHABLEEXCEPTION_H
#define DIGITWESTER_SERVERNOTREACHABLEEXCEPTION_H

#include <exception>

namespace REALTWESTER {
    namespace EXCEPTIONS {
        class ServerNotReachableException : public std::exception{
        public:
            ServerNotReachableException();
            virtual ~ServerNotReachableException() = default;

            const char * what() const noexcept override;
        };
    }
}

#endif //DIGITWESTER_SERVERNOTREACHABLEEXCEPTION_H
