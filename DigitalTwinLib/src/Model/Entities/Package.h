#pragma once

#include <map>
#include <vector>

#include "ICollectionType.h"
#include "IDigitalTwinElement.h"

namespace DigitalTwin::Model {
	class Component;
	class Port;

	class Package : public ICollectionType
	{
	public:
		Package() = delete;
		Package(std::string name);

		virtual ~Package() = default;

		void appendComponent(Component* component) override;
		void appendPort(Port* port) override;
		void appendAttribute(IVariable* variable) override;
		void appendMeasurable(IVariable* variable) override;
		void appendControllable(IVariable* variable) override;
		void instantiateComponent(std::string instanceName, std::string componentName);

		Component* getComponentDefinition(std::string name);
		Port* getPort(std::string name);
		IVariable* resolveVariable(std::string name) override;
		IVariable* resolveVariable(std::vector<std::string> domains, size_t index) override;
		IVariable* getMeasurable(std::string name);
		IVariable* getControllable(std::string name);
		Component* getIndividualInstance(std::string name);

		std::vector<std::string> getAllMQTTTopics();

	private:
		std::map<std::string, Component*> ComponentDefinitions;
		std::map<std::string, Port*> PortDefinitions;
		std::map<std::string, Component*> IndividualInstances;
	};
}