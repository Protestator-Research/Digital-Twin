#include "Package.h"

#include "Component.h"
#include "Port.h"
#include "../Exceptions/DigitalTwinAddressException.h"
#include "Variable.h"
#include "BaseFuctions/StringExtention.hpp"

namespace DigitalTwin::Model
{
	Package::Package(std::string name) : ICollectionType(name) {}

	void Package::appendComponent(Component* component)
	{
		ComponentDefinitions.insert(std::make_pair(component->getName(), component));
	}

	void Package::appendPort(Port* port)
	{
		PortDefinitions.insert(std::make_pair(port->getName(), port));
	}

	void Package::appendAttribute(Variable* variable)
	{
		Attributes.insert(std::make_pair(variable->getName(), variable));
	}

	void Package::appendMeasurable(Variable* variable)
	{
		Measurables.insert(std::make_pair(variable->getName(), variable));
	}

	void Package::appendControllable(Variable* variable)
	{
		Controllables.insert(std::make_pair(variable->getName(), variable));
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

	Variable* Package::resolveVariable(std::string name)
	{
		auto splittedAdress = CPSBASELIB::STD_EXTENTION::StringExtention::splitString(name, '/');

		if (splittedAdress.size() == 1)
			splittedAdress = CPSBASELIB::STD_EXTENTION::StringExtention::splitString(name, '.');

		return resolveVariable(splittedAdress, 0);
	}

	Variable* Package::resolveVariable(std::vector<std::string> domains, int index)
	{
		if (index >= domains.size())
			throw DigitalTwinAddressException();

		if ((size_t)index == (domains.size() - 1))
		{
			if (Controllables.contains(domains.back()))
				return Controllables.at(domains.back());
			if (Measurables.contains(domains.back()))
				return Measurables.at(domains.back());
			if (Attributes.contains(domains.back()))
				return Attributes.at(domains.back());
			throw DigitalTwinAddressException();
		}

		return dynamic_cast<Component*>(ComponentDefinitions[domains[index]])->resolveVariable(domains, index + 1);
	}

	Variable* Package::getMeasurable(std::string name)
	{
		return Measurables.at(name);
	}

	Variable* Package::getControllable(std::string name)
	{
		return Controllables.at(name);
	}

	Component* Package::getIndividualInstance(std::string name)
	{
		return IndividualInstances.at(name);
	}

	std::vector<std::string> Package::getAllMQTTTopics()
	{
		std::vector<std::string> returnValue;

		for (const auto& [name, _] : Controllables)
			returnValue.push_back(getName() + "/" + name);
		
		for (const auto& [name, _] : Measurables)
			returnValue.push_back(getName() + "/" + name);

		for (const auto& [_, instance] : IndividualInstances)
			for (const auto& topic : instance->getAllMQTTTopics())
				returnValue.push_back(getName() + "/" + topic);

		return returnValue;
	}
}
