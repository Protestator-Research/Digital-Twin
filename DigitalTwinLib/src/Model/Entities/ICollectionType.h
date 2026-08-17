#pragma once

#include <string>
#include <vector>
#include <map>
#include <any>
#include "IDigitalTwinElement.h"
#include "../../cpp_digital_twin_lib_global.h"
#include "Variable.h"

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
        virtual void appendAttribute(Variable<std::any>* variable) = 0;
        virtual void appendControllable(Variable<std::any>* variable) = 0;
        virtual void appendMeasurable(Variable<std::any>* variable) = 0;

        virtual Variable<std::any>* resolveVariable(std::string name) = 0;
        virtual Variable<std::any>* resolveVariable(std::vector<std::string> domains, size_t index) = 0;

    protected:
    	std::map<std::string, Component*> ComponentMap;
    	std::map<std::string, Port*> PortMap;
    	std::map<std::string, Variable<std::any>*> Controllables;
    	std::map<std::string, Variable<std::any>*> Measurables;
    	std::map<std::string, Variable<std::any>*> Attributes;
        
    };
}

