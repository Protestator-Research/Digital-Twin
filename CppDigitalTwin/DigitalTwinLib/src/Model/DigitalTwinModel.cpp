//
// Created by Moritz Herzog on 23.05.24.
//

#include <SysMLv2Standard/entities/DigitalTwin.h>
#include <SysMLv2Standard/entities/DataIdentity.h>

#include "DigitalTwinModel.h"
#include "../DigitalTwinManager.h"

namespace DigitalTwin::Model {
    DigitalTwinModel::DigitalTwinModel(SysMLv2::Entities::DigitalTwin *digitalTwin, DigitalTwinManager *manager) :
            DigitalTwin(digitalTwin),
            Manager(manager)
    {

    }

    void DigitalTwinModel::generateDigitalTwinBackend() {
        auto elements = Manager->downloadDigitalTwinModel(DigitalTwin->parentProjectId()->getId(), DigitalTwin->commitId()->getId());
    }
}