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
#include <boost/uuid/uuid.hpp>
//---------------------------------------------------------
// Internal Classes
//---------------------------------------------------------
#include "Record.h"

//---------------------------------------------------------
// Forwarding
//---------------------------------------------------------
namespace SysMLv2::Entities {
    class DataIdentity;
    class Data;
}


namespace SysMLv2::Entities {
    /**
     * The class DataVersion represents a specific version of the Data in its lifecycle
     * @class DataVersion
     * @author Moritz Herzog <herzogm@rptu.de>
     * @version 1.0
     */
    class DataVersion : public Record {
    public:
        DataVersion() = delete;

        DataVersion(DataIdentity* identity, Data* payload = nullptr);
        virtual ~DataVersion();

        DataIdentity* getIdentity();
        Data* getPayload();
        boost::uuids::uuid getId();

    private:
        DataIdentity* Identity;

        Data* Payload;
    };
}

#endif //DIGITALTWIN_DATAVERSION_H
