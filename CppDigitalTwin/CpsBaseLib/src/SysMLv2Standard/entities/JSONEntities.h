//
// Created by Moritz Herzog on 22.02.24.
//
//---------------------------------------------------------
// Constants, Definitions, Pragmas
//---------------------------------------------------------
#ifndef DIGITALTWIN_JSONENTITIES_H
#define DIGITALTWIN_JSONENTITIES_H
//---------------------------------------------------------
// External Classes
//---------------------------------------------------------
#include <string>
//---------------------------------------------------------
// Internal Classes
//---------------------------------------------------------

//---------------------------------------------------------
// Forwarding
//---------------------------------------------------------

namespace SysMLv2::Entities {
    //Generally used
    const std::string JSON_ID_ENTITY = "@id";
    //Used for Record and the Superclasses
    const std::string JSON_TYPE_ENTITY = "@type";
    const std::string JSON_NAME_ENTITY = "name";
    const std::string JSON_ALIAS_ENTITY = "alias";
    const std::string JSON_DESCRIPTION_ENTITY = "description";
    //Used for Project
    const std::string JSON_DEFAULT_BRANCH_ENTITY = "defaultBranch";
    const std::string JSON_CREATION = "created";

}


#endif //DIGITALTWIN_JSONENTITIES_H
