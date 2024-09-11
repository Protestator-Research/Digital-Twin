//
// Created by Moritz Herzog on 24.04.24.
//

#ifndef DIGITALTWIN_STATUSCODE_H
#define DIGITALTWIN_STATUSCODE_H


#include "../cpp_backend_communication_global.h"

namespace BACKEND_COMMUNICATION {
    enum CPSBACKENDCOMMUNICATION_EXPORT INTERNAL_STATUS_CODE {
        SUCCESS = 0,
        RE_DO,
        THROW_ERROR
    };
}

#endif //DIGITALTWIN_STATUSCODE_H
