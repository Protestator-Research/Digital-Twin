//
// Created by Moritz Herzog on 19.02.24.
//
//---------------------------------------------------------
// Constants, Definitions, Pragmas
//---------------------------------------------------------
#ifndef DIGITALTWIN_IENTITY_H
#define DIGITALTWIN_IENTITY_H
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
    /**
     * Defines the interface for all SysMLv2 classes.
     * @class IEntity
     * @author Moritz Herzog <herzogm@rptu.de>
     * @version 1.0
     */
    class IEntity {
    public:

        /**
         * Allows the serialization of the Object to a JSON String.
         * @return JSON String of the Object
         */
        virtual std::string serializeToJson() = 0;
    };
}


#endif //DIGITALTWIN_IENTITY_H
