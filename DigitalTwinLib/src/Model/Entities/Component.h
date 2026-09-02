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
        void appendAttribute(IVariable* variable) override;
        void appendControllable(IVariable* variable) override;
        void appendMeasurable(IVariable* variable) override;


        IVariable* getControllable(std::string name);
        IVariable* getMeasurable(std::string name);
        IVariable* getAttribute(std::string name);
        IVariable* resolveVariable(std::string name) override;
        IVariable* resolveVariable(std::vector<std::string> domains, size_t index) override;
        Component* getComponent(std::string name);
        Port* getPort(std::string name);

        std::vector<Component*> getAllComponents();
        std::vector<IVariable*> getAllVariables();
        std::vector<Port*> getAllPorts();

        std::vector<std::string> getAllMQTTTopics();

        Component* instantiate(std::string name);

    private:

    };
}

#endif //DIGITALTWIN_COMPONENT_H
