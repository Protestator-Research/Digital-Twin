//
// Created by moritz-herzog on 08.08.23.
//

#include "NotInitializedException.h"

REALTWESTER::EXCEPTION::NotInitializedException::NotInitializedException(std::string &name) : exception() {
    Name = name;
}

const char *REALTWESTER::EXCEPTION::NotInitializedException::what() const noexcept {
    std::string what = "Class with " + Name + "is not initialized";
    return what.c_str();
}
