//
// Created by Moritz Herzog on 07.08.24.
//

#ifndef DIGITALTWIN_JSONENTITIES_HPP
#define DIGITALTWIN_JSONENTITIES_HPP

#include <string>

#include "../../cpp_physical_twin_communication_global.h"

namespace DigitalTwin::Communication {
    class CPPPHYSICALTWINCOMMUNICATION_EXPORT JsonEntities {
    public:
        static const char* DIGITAL_TWIN_ID_ENTITY;
        static const char* PROJECT_ID_ENTITY;
        static const char* VALUE_ENTITY;
        static const char* TIME_POINT_ENTITY;
    };
}

#endif //DIGITALTWIN_JSONENTITIES_HPP
