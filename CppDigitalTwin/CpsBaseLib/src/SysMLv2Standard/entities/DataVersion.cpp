//
// Created by Moritz Herzog on 23.02.24.
//

#include "DataVersion.h"
#include "DataIdentity.h"
#include "Data.h"


namespace SysMLv2::Entities {
    DataVersion::DataVersion(DataIdentity *identity) : Record(identity->getId()) {
        Identity = identity;
        Payload = new Data();
    }

    DataVersion::~DataVersion() {
        delete Identity;
        delete Payload;
    }

    DataIdentity *DataVersion::getIdentity() {
        return Identity;
    }

    Data *DataVersion::getPayload() {
        return Payload;
    }

    boost::uuids::uuid DataVersion::getId() {
        return Payload->getId();
    }


}