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
        void appendAttribute(Variable<std::any>* variable) override;
        void appendControllable(Variable<std::any>* variable) override;
        void appendMeasurable(Variable<std::any>* variable) override;


        Variable<std::any>* getControllable(std::string name);
        Variable<std::any>* getMeasurable(std::string name);
        Variable<std::any>* getAttribute(std::string name);
        Variable<std::any>* resolveVariable(std::string name) override;
        Variable<std::any>* resolveVariable(std::vector<std::string> domains, size_t index) override;
        Component* getComponent(std::string name);
        Port* getPort(std::string name);

        std::vector<Component*> getAllComponents();
        std::vector<Variable<std::any>*> getAllVariables();
        std::vector<Port*> getAllPorts();

        std::vector<std::string> getAllMQTTTopics();

        Component* instantiate(std::string name);

    private:

    };
}

#endif //DIGITALTWIN_COMPONENT_H
