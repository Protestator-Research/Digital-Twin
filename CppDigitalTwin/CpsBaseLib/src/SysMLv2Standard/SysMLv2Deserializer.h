//
// Created by Moritz Herzog on 29.02.24.
//
//---------------------------------------------------------
// Constants, Definitions, Pragmas
//---------------------------------------------------------
#ifndef DIGITALTWIN_SYSMLV2DESERIALIZER_H
#define DIGITALTWIN_SYSMLV2DESERIALIZER_H
//---------------------------------------------------------
// External Classes
//---------------------------------------------------------
#include <memory>
#include <vector>
//---------------------------------------------------------
// Internal Classes
//---------------------------------------------------------
#include "entities/IEntity.h"

namespace SysMLv2 {

    class SysMLv2Deserializer {
    public:
        static SysMLv2::Entities::IEntity* deserializeJsonString(std::string inputValue);
        static std::vector<SysMLv2::Entities::IEntity*> deserializeJsonArray(std::string inputValue);
    };

} // SysMLv2

#endif //DIGITALTWIN_SYSMLV2DESERIALIZER_H
