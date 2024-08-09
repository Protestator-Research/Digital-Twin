//
// Created by Moritz Herzog on 23.05.24.
//

#ifndef DIGITALTWIN_DIGITALTWINMODEL_H
#define DIGITALTWIN_DIGITALTWINMODEL_H

#include <vector>

namespace SysMLv2::Entities {
    class DigitalTwin;
    class Data;
}


namespace DigitalTwin::Model {
    class DigitalTwinModel {
    public:
        DigitalTwinModel() = delete;
        explicit DigitalTwinModel(SysMLv2::Entities::DigitalTwin* digitalTwin);
        virtual ~DigitalTwinModel() = default;

        void generateDigitalTwinBackend();
    private:
        [[maybe_unused]] SysMLv2::Entities::DigitalTwin* DigitalTwin;
        std::vector<SysMLv2::Entities::Data*> DigitalTwinModelElements;
    };
}

#endif //DIGITALTWIN_DIGITALTWINMODEL_H
