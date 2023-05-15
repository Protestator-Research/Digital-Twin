//
// Created by moritzh on 12.05.23.
//

#include "ServerNotReachableException.h"

namespace REALTWESTER {
    namespace EXCEPTIONS {
        ServerNotReachableException::ServerNotReachableException()
            : std::exception() {
        }

        const char* ServerNotReachableException::what() const noexcept {
            return "Given Server could not be reached. Please give a correct ip-adress or domain.";
        }
    }
}