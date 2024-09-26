//
// Created by Moritz Herzog on 23.05.24.
//

#include <SysMLv2Standard/entities/DigitalTwin.h>
#include <SysMLv2Standard/entities/DataIdentity.h>
#include <SysMLv2Standard/entities/Element.h>

#include "DigitalTwinModel.h"
#include "Entities/IDigitalTwinElement.h"
#include "Entities/Component.h"
#include "Entities/Connection.h"
#include "Entities/Variable.h"
#include "Entities/Port.h"
#include "../DigitalTwinManager.h"
#include "Parser/Parser.h"

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

        auto digitalTwinElements = Parser::Parser::parse(Parser::SupportedModels::SysMLv2,completeModel);
        for(auto dtElement : digitalTwinElements) {
            ComponentMap.insert(std::make_pair(dtElement->getName(),dtElement));
        }

    }

    std::string DigitalTwinModel::digitalTwinName() {
        return DigitalTwin->getName();
    }
}