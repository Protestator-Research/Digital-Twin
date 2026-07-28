#pragma once

#include <map>
#include <vector>

#include "ICollectionType.h"
#include "IDigitalTwinElement.h"

namespace DigitalTwin::Model {
	class Component;
	class Variable;
	class Port;

	class Package : public ICollectionType
	{
	public:
		Package() = delete;
		Package(std::string name);

		virtual ~Package() = default;

		void appendComponent(Component* component) override;
		void appendPort(Port* port) override;
		void appendAttribute(Variable* variable) override;
		void appendMeasurable(Variable* variable) override;
		void appendControllable(Variable* variable) override;
		void instantiateComponent(std::string instanceName, std::string componentName);

		Component* getComponentDefinition(std::string name);
		Port* getPort(std::string name);
		Variable* resolveVariable(std::string name) override;
		Variable* resolveVariable(std::vector<std::string> domains, size_t index) override;
		Variable* getMeasurable(std::string name);
		Variable* getControllable(std::string name);
		Component* getIndividualInstance(std::string name);

		std::vector<std::string> getAllMQTTTopics();

	private:
		std::map<std::string, Component*> ComponentDefinitions;
		std::map<std::string, Port*> PortDefinitions;
		std::map<std::string, Variable*> Attributes;
		std::map<std::string, Variable*> Measurables;
		std::map<std::string, Variable*> Controllables;
		std::map<std::string, Component*> IndividualInstances;
	};
}