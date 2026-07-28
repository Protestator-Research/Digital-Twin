//
// Created by Moritz Herzog on 08.08.24.
//

#ifndef DIGITALTWIN_COMPONENT_H
#define DIGITALTWIN_COMPONENT_H

#include <string>
#include <vector>
#include <map>

#include "ICollectionType.h"
#include "../../cpp_digital_twin_lib_global.h"

namespace DigitalTwin::Model {
    class Variable;
}

namespace DigitalTwin::Model {
    /**
     *
     */
    class CPPDIGITALTWINLIB_EXPORT Component : public ICollectionType {
    public:
        /**
         * CTor is Deleted
         */
        Component() = delete;
        /**
         *
         * @param name
         */
        explicit Component(std::string name);

        ~Component() override;

        void appendPort(Port* port) override;
        void appendComponent(Component* component) override;
        void appendAttribute(Variable* variable) override;
        void appendControllable(Variable* variable) override;
        void appendMeasurable(Variable* variable) override;


        Variable* getControllable(std::string name);
        Variable* getMeasurable(std::string name);
        Variable* getAttribute(std::string name);
        Variable* resolveVariable(std::string name) override;
        Variable* resolveVariable(std::vector<std::string> domains, size_t index) override;
        Component* getComponent(std::string name);
        Port* getPort(std::string name);

        std::vector<Component*> getAllComponents();
        std::vector<Variable*> getAllVariables();
        std::vector<Port*> getAllPorts();

        std::vector<std::string> getAllMQTTTopics();

        Component* instantiate(std::string name);

    private:
        std::map<std::string, Component*> ComponentMap;
        std::map<std::string, Port*> PortMap;
    	std::map<std::string, Variable*> Controllables;
        std::map<std::string, Variable*> Measurables;
        std::map<std::string, Variable*> Attributes;
    };
}

#endif //DIGITALTWIN_COMPONENT_H
