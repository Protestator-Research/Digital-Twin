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

		void addComponentDefinition(Component* component);
		void addPortDefinition(Port* port);
		void appendAttribute(Variable* variable) override;
		void appendMeasurable(Variable* variable) override;
		void appendControllable(Variable* variable) override;
		void instantiateComponent(std::string instanceName, std::string componentName);

		Component* getComponentDefinition(std::string name);
		Port* getPort(std::string name);
		Variable* getVariable(std::string name);
		Variable* getMeasurable(std::string name);
		Variable* getControllable(std::string name);
		Component* getIndividualInstance(std::string name);

		std::vector<std::string> getAllMQTTTopics();

	private:
		std::map<std::string, Component*> ComponentDefinitions;
		std::map<std::string, Port*> PortDefinitions;
		std::map<std::string, Variable*> Variables;
		std::map<std::string, Variable*> Measurables;
		std::map<std::string, Variable*> Controllebles;
		std::map<std::string, Component*> IndividualInstances;
	};
}