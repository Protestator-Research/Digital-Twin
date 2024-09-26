//
// Created by Moritz Herzog on 08.08.24.
//

#ifndef DIGITALTWIN_COMPONENT_H
#define DIGITALTWIN_COMPONENT_H

#include <string>
#include <vector>
#include <map>

#include "IDigitalTwinElement.h"
namespace DigitalTwin::Model {
    class Variable;
}

namespace DigitalTwin::Model {
    /**
     *
     */
    class Component : public IDigitalTwinElement{
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

        void appendComponent(Component* component);
        void appendAttribute(Variable* variable);
        void appendControllable(Variable* variable);
        void appendMeasurable(Variable* variable);


        Variable* getControllable(std::string name);
        Variable* getMeasurable(std::string name);
        Variable* getAttribute(std::string name);
        Component* getComponent(std::string name);


    private:
        std::map<std::string, Component*> ComponentMap;
        std::map<std::string, Variable*> Controllables;
        std::map<std::string, Variable*> Measurables;
        std::map<std::string, Variable*> Attributes;
    };
}

#endif //DIGITALTWIN_COMPONENT_H
