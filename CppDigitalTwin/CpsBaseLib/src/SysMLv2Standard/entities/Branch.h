//
// Created by Moritz Herzog on 23.02.24.
//
//---------------------------------------------------------
// Constants, Definitions, Pragmas
//---------------------------------------------------------
#ifndef DIGITALTWIN_BRANCH_H
#define DIGITALTWIN_BRANCH_H
//---------------------------------------------------------
// External Classes
//---------------------------------------------------------

//---------------------------------------------------------
// Internal Classes
//---------------------------------------------------------
#include "CommitReference.h"

//---------------------------------------------------------
// Forwarding
//---------------------------------------------------------

namespace SysMLv2::Entities {

    /**
     * Represents an abstract baseclass that is used for the SysMLv2 API
     * @class Record
     * @author Moritz Herzog <herzogm@rptu.de>
     * @version 1.0
     */
    class Branch : public CommitReference{
    public:
        /**
         *
         * @param jsonStringOrName
         */
        Branch(std::string jsonStringOrName);

        /**
         * Compares the two Branches
         * @param other
         * @return
         */
        bool operator==(Branch& other);

        std::string serializeToJson() override;
    };
}

#endif //DIGITALTWIN_BRANCH_H
