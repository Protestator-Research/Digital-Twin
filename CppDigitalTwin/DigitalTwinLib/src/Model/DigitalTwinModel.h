//
// Created by Moritz Herzog on 23.05.24.
//

#ifndef DIGITALTWIN_DIGITALTWINMODEL_H
#define DIGITALTWIN_DIGITALTWINMODEL_H

#include <vector>
#include <string>

#include "../cpp_digital_twin_lib_global.h"

namespace SysMLv2::Entities {
    class DigitalTwin;
    class Element;
}

namespace DigitalTwin {
    class DigitalTwinManager;
}

namespace DigitalTwin::Model {
    class CPPDIGITALTWINLIB_EXPORT DigitalTwinModel {
    public:
        DigitalTwinModel() = delete;
        explicit DigitalTwinModel(SysMLv2::Entities::DigitalTwin* digitalTwin, DigitalTwinManager* manager);
        virtual ~DigitalTwinModel();

        void generateDigitalTwinBackend();

        std::string digitalTwinName();

    private:
        SysMLv2::Entities::DigitalTwin* DigitalTwin;
        std::vector<SysMLv2::Entities::Element*> DigitalTwinModelElements;
        DigitalTwinManager* Manager;
    };
}

#endif //DIGITALTWIN_DIGITALTWINMODEL_H
