//
// Created by moritz-herzog on 08.08.23.
//

#ifndef DIGITALTWINKI4BOARDNET_NOTINITIALIZEDEXCEPTION_H
#define DIGITALTWINKI4BOARDNET_NOTINITIALIZEDEXCEPTION_H

#include <exception>
#include <string>

namespace REALTWESTER::EXCEPTION {
    class NotInitializedException : public std::exception {
    public:
        NotInitializedException(std::string &name);

        const char * what() const noexcept override;

    private:
        std::string Name = "";
    };
}

#endif //DIGITALTWINKI4BOARDNET_NOTINITIALIZEDEXCEPTION_H
