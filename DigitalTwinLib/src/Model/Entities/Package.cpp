#include "Package.h"

#include "Component.h"
#include "Port.h"
#include "Variable.h"

namespace DigitalTwin::Model
{
	Package::Package(std::string name) : ICollectionType(name) {}

	void Package::addComponentDefinition(Component* component)
	{
		ComponentDefinitions.insert(std::make_pair(component->getName(), component));
	}

	void Package::addPortDefinition(Port* port)
	{
		PortDefinitions.insert(std::make_pair(port->getName(), port));
	}

	void Package::appendAttribute(Variable* variable)
	{
		Variables.insert(std::make_pair(variable->getName(), variable));
	}

	void Package::appendMeasurable(Variable* variable)
	{
		Measurables.insert(std::make_pair(variable->getName(), variable));
	}

	void Package::appendControllable(Variable* variable)
	{
		Controllebles.insert(std::make_pair(variable->getName(), variable));
	}

	void Package::instantiateComponent(std::string instanceName, std::string componentName)
	{
		const auto componentInstance = ComponentDefinitions.at(componentName)->instantiate(instanceName);
		ComponentDefinitions.insert(std::make_pair(instanceName, componentInstance));
	}

	Component* Package::getComponentDefinition(std::string name)
	{
		return ComponentDefinitions.at(name);
	}

	Port* Package::getPort(std::string name)
	{
		return PortDefinitions.at(name);
	}

	Variable* Package::getVariable(std::string name)
	{
		return Variables.at(name);
	}

	Variable* Package::getMeasurable(std::string name)
	{
		return Measurables.at(name);
	}

	Variable* Package::getControllable(std::string name)
	{
		return Controllebles.at(name);
	}

	Component* Package::getIndividualInstance(std::string name)
	{
		return IndividualInstances.at(name);
	}

	std::vector<std::string> Package::getAllMQTTTopics()
	{
		std::vector<std::string> returnValue;

		for (const auto& [name, _] : Controllebles)
			returnValue.push_back(getName() + "/" + name);
		
		for (const auto& [name, _] : Measurables)
			returnValue.push_back(getName() + "/" + name);

		for (const auto& [_, instance] : IndividualInstances)
			for (const auto& topic : instance->getAllMQTTTopics())
				returnValue.push_back(getName() + "/" + topic);

		return returnValue;
	}
}
