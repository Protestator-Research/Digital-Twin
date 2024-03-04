//
// Created by Moritz Herzog on 23.02.24.
//
//---------------------------------------------------------
// Constants, Definitions, Pragmas
//---------------------------------------------------------
#ifndef DIGITALTWIN_DATAVERSION_H
#define DIGITALTWIN_DATAVERSION_H
//---------------------------------------------------------
// External Classes
//---------------------------------------------------------

//---------------------------------------------------------
// Internal Classes
//---------------------------------------------------------
#include "Record.h"

//---------------------------------------------------------
// Forwarding
//---------------------------------------------------------
namespace SysMLv2::Entities {
    class DataIdentity;
}


namespace SysMLv2::Entities {
    /**
     * The class DataVersion represets a specific version of the Data in its lifecycle
     * @class DataVersion
     * @author Moritz Herzog <herzogm@rptu.de>
     * @version 1.0
     */
    class DataVersion : public Record {
    public:

    private:
        [[maybe_unused]] DataIdentity* Identity;
    };
}

#endif //DIGITALTWIN_DATAVERSION_H
