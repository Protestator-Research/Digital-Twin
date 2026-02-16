//
// Created by Moritz Herzog on 23.05.24.
//

#ifndef DIGITALTWIN_DIGITALTWINMODEL_H
#define DIGITALTWIN_DIGITALTWINMODEL_H

#include <vector>
#include <string>
#include <map>
#include <functional>
#include <boost/uuid/uuid.hpp>

#include "../cpp_digital_twin_lib_global.h"

namespace SysMLv2::REST {
    class DigitalTwin;
}

namespace KerML::Entities {
    class Element;
}

namespace DigitalTwin {
    class DigitalTwinManager;
    namespace Model{
        class IDigitalTwinElement;
        class Component;
        class Variable;
    }
}

namespace DigitalTwin::Model {
    class CPPDIGITALTWINLIB_EXPORT DigitalTwinModel {
    public:
        DigitalTwinModel() = delete;
        explicit DigitalTwinModel(std::shared_ptr<SysMLv2::REST::DigitalTwin> digitalTwin, DigitalTwinManager* manager);
        virtual ~DigitalTwinModel();

        void generateDigitalTwinBackend();

        std::string digitalTwinName();

        std::vector<IDigitalTwinElement*> getAllComponents() const;

        Variable* getVariableWithAddress(std::string address);
        Component* getComponentWithAddress(std::string address);

        std::vector<std::string> getElementStrings();

        void setUpdateModelFunction(std::function<void()> updateModel);
    private:
        std::shared_ptr<SysMLv2::REST::DigitalTwin> DigitalTwin;
        std::vector<std::shared_ptr<KerML::Entities::Element>> DigitalTwinModelElements;
        [[maybe_unused]] DigitalTwinManager* Manager;
        std::map<std::string, IDigitalTwinElement*> ComponentMap;
        std::map<std::string, IDigitalTwinElement*> PortMap;
        std::function<void()> UpdateModelFunction;
    };
}

#endif //DIGITALTWIN_DIGITALTWINMODEL_H
