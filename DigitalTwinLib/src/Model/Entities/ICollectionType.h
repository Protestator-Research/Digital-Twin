#pragma once

#include <string>
#include <vector>
#include <map>
#include <any>
#include "IDigitalTwinElement.h"
#include "../../cpp_digital_twin_lib_global.h"
#include "Variables/Variable.hpp"

namespace DigitalTwin::Model {
	class Port;
	class Component;
    /**
     * @class ICollectionType
     * @author Moritz Herzog
     * @version 1.0
     * This Class represents the BaseClass of the Elements of the Digital Twin.
     * If there are parts that are needed across all DT Elements, this holds it.
     */
    class CPPDIGITALTWINLIB_EXPORT ICollectionType : public IDigitalTwinElement{
    public:
        /**
         * Is deleted, because there is no need.
         */
        ICollectionType() = delete;

        /**
         * Constructor that initializes the name of the Element. The name can never be changed,
         * because it is a core component in the
         * @param name Name of a DT Element
         */
        explicit ICollectionType(std::string name) : IDigitalTwinElement(name) { }

        virtual void appendComponent(Component* compoonent) = 0;
        virtual void appendPort(Port* port) = 0;
        virtual void appendAttribute(IVariable* variable) = 0;
        virtual void appendControllable(IVariable* variable) = 0;
        virtual void appendMeasurable(IVariable* variable) = 0;

        virtual IVariable* resolveVariable(std::string name) = 0;
        virtual IVariable* resolveVariable(std::vector<std::string> domains, size_t index) = 0;

    protected:
    	std::map<std::string, Component*> ComponentMap;
    	std::map<std::string, Port*> PortMap;
    	std::map<std::string, IVariable*> Controllables;
    	std::map<std::string, IVariable*> Measurables;
    	std::map<std::string, IVariable*> Attributes;
        
    };
}

