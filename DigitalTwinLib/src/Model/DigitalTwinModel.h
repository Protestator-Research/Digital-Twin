//
// Created by Moritz Herzog on 23.05.24.
//

#ifndef DIGITALTWIN_DIGITALTWINMODEL_H
#define DIGITALTWIN_DIGITALTWINMODEL_H

#include <vector>
#include <string>
#include <map>
#include <any>
#include <functional>
#include <boost/uuid/uuid.hpp>

#include "../cpp_digital_twin_lib_global.h"
#include "Entities/Variables/Variable.hpp"
#include <sysmlv2/service/implementation/InstanceManager.h>

namespace SysMLv2::REST {
    class DigitalTwin;
}

namespace KerML::Entities {
    class Element;
    class NamespaceImport;
}

namespace DigitalTwin {
    class DigitalTwinManager;
    namespace Model{
	    class Package;
	    class Port;
	    class IDigitalTwinElement;
        class ICollectionType;
        class Component;
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
        std::vector<IDigitalTwinElement*> getAllPackages() const;

        IVariable* getVariableWithAddress(std::string address);
        Component* getComponentWithAddress(std::string address);

        std::vector<std::string> getElementStrings();

        void setUpdateModelFunction(std::function<void()> updateModel);


    private:
        void buildDigitalTwinModel();

        void generateDigitalTwinModelRecursively(const std::shared_ptr<KerML::Entities::Element>& element, ICollectionType* parent);


        std::shared_ptr<SysMLv2::REST::DigitalTwin> DigitalTwin;
        std::vector<std::shared_ptr<KerML::Entities::Element>> DigitalTwinModelElements;
        std::vector<std::shared_ptr<KerML::Entities::Element>> RootElements;
        [[maybe_unused]] DigitalTwinManager* Manager;
        std::map<std::string, Component*> ComponentMap;
        std::map<std::string, Package*> PackageMap;
        std::map<std::string, Port*> PortMap;
        std::function<void()> UpdateModelFunction;
        SysMLv2::API::InstanceManager* Instance;
    };
}

#endif //DIGITALTWIN_DIGITALTWINMODEL_H
