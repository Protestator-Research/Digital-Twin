#include "ModelTree.h"

#include <set>

#include "../Entities/Component.h"
#include "../Entities/ICollectionType.h"
#include "../Entities/Package.h"
#include "../Entities/Port.h"
#include "../Entities/Variables/Variable.hpp"

namespace DigitalTwin::Model {
	namespace {
		class TreeWalker {
		public:
			std::vector<ModelTreeEntry> Entries;

			void addComponent(const std::string& parentPath, Component* component)
			{
				addCollection(parentPath, component, ElementKind::Component);
			}

			void addPackage(Package* package)
			{
				addCollection(std::string(), package, ElementKind::Package);
			}

			void addPort(const std::string& parentPath, Port* port)
			{
				addCollection(parentPath, port, ElementKind::Port);
			}

		private:
			std::set<std::string> UsedPaths;

			static std::string join(const std::string& parentPath, const std::string& name)
			{
				return parentPath.empty() ? name : parentPath + "/" + name;
			}

			// Returns false if the path is taken.
			bool addEntry(const std::string& parentPath, const std::string& name, ElementKind kind, IVariable* variable,
			              std::string& pathOut)
			{
				pathOut = join(parentPath, name);
				if (!UsedPaths.insert(pathOut).second)
					return false;
				ModelTreeEntry entry;
				entry.info.path = pathOut;
				entry.info.parentPath = parentPath;
				entry.info.name = name;
				entry.info.kind = kind;
				if (variable)
					entry.info.valueType = variable->getValueType();
				entry.variable = variable;
				Entries.push_back(std::move(entry));
				return true;
			}

			void addVariables(const std::string& path, const std::map<std::string, IVariable*>& variables, ElementKind kind)
			{
				for (const auto& [name, variable] : variables)
				{
					if (!variable)
						continue;
					std::string variablePath;
					addEntry(path, name, kind, variable, variablePath);
				}
			}

			void addCollection(const std::string& parentPath, ICollectionType* collection, ElementKind kind)
			{
				if (!collection)
					return;
				std::string path;
				if (!addEntry(parentPath, collection->getName(), kind, nullptr, path))
					return;

				addVariables(path, collection->getControllableMap(), ElementKind::Controllable);
				addVariables(path, collection->getMeasurableMap(), ElementKind::Measurable);
				addVariables(path, collection->getAttributeMap(), ElementKind::Attribute);

				if (kind == ElementKind::Package)
				{
					for (const auto instance : static_cast<Package*>(collection)->getAllInstances())
						addComponent(path, instance);
					return;
				}
				for (const auto& [name, component] : collection->getComponentMap())
					addComponent(path, component);
				for (const auto& [name, port] : collection->getPortMap())
					addPort(path, port);
			}
		};
	}

	std::vector<ModelTreeEntry> walkModelTree(const std::map<std::string, Package*>& packages,
	                                          const std::map<std::string, Component*>& components,
	                                          const std::map<std::string, Port*>& ports)
	{
		TreeWalker walker;
		// Components first: getVariableWithAddress looks in the components before the packages.
		for (const auto& [name, component] : components)
			walker.addComponent(std::string(), component);
		for (const auto& [name, package] : packages)
			walker.addPackage(package);
		for (const auto& [name, port] : ports)
			walker.addPort(std::string(), port);
		return std::move(walker.Entries);
	}
}
