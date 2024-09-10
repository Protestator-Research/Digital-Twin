//
// Created by Moritz Herzog on 23.05.24.
//

#include <SysMLv2Standard/entities/DigitalTwin.h>
#include <SysMLv2Standard/entities/DataIdentity.h>
#include <SysMLv2Standard/entities/Element.h>

#include "DigitalTwinModel.h"
#include "../DigitalTwinManager.h"

namespace DigitalTwin::Model {
    DigitalTwinModel::DigitalTwinModel(SysMLv2::Entities::DigitalTwin *digitalTwin, DigitalTwinManager *manager) :
            DigitalTwin(digitalTwin),
            Manager(manager)
    {
        generateDigitalTwinBackend();
    }

    DigitalTwinModel::~DigitalTwinModel() {

    }

    void DigitalTwinModel::generateDigitalTwinBackend() {
        auto allElements = Manager->downloadDigitalTwinModel(DigitalTwin->parentProjectId()->getId(), DigitalTwin->commitId()->getId());

        for(const auto item : DigitalTwin->getConnectedModels())
            for(const auto elem : allElements)
                if(item->getId()==elem->getId())
                    DigitalTwinModelElements.push_back(elem);

        std::string completeModel;

        for(const auto elem : DigitalTwinModelElements)
            completeModel+=elem->body();


    }

    std::string DigitalTwinModel::digitalTwinName() {
        return DigitalTwin->getName();
    }
}