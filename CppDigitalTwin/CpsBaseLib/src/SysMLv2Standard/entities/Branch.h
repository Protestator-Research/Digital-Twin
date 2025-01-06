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
#include "../../cps_base_global.h"
//---------------------------------------------------------
// Forwarding
//---------------------------------------------------------

namespace SysMLv2::Entities {
    class Commit;

    /**
     * Represents an Branch that is used for the SysMLv2 API
     * @class Branch
     * @author Moritz Herzog <herzogm@rptu.de>
     * @version 1.0
     */
    class CPSBASELIB_EXPORT Branch : public CommitReference{
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

        /**
         * Returns the Current head of the given Branch.
         * @return A Commit
         * @see SysMLv2::Entities::Commit
         */
        Commit* getHead();

    private:
        Commit* Head;
    };
}

#endif //DIGITALTWIN_BRANCH_H
