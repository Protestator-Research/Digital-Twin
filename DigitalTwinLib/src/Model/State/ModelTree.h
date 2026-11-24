// Walk of the tree of a digital twin model. A free function on the root maps of the model, so that it can be tested
// on hand-built Package / Component objects (a DigitalTwinModel needs a backend to be constructed).
#pragma once

#include <map>
#include <string>
#include <vector>

#include "StateTypes.h"
#include "../../cpp_digital_twin_lib_global.h"

namespace DigitalTwin::Model {
	class Component;
	class IVariable;
	class Package;
	class Port;

	struct ModelTreeEntry {
		ModelElementInfo info;
		IVariable* variable = nullptr;   // set for variables only, not owned
	};

	/**
	 * Lists the elements of the model: Packages, Components, Ports and all variables (Controllables, Measurables
	 * and Attributes), parents before children, in the order of the maps (sorted by name).
	 *
	 * The path of a variable equals the MQTT topic path that DigitalTwinModel::getElementValueEntities uses for it
	 * (without the function suffixes). Where the MQTT interface has no path (attributes, ports) the path is built
	 * the same way: <path of the owner>/<name>. A package contributes its own variables and its component
	 * instances; its component and port definitions are templates without a state and are not listed.
	 *
	 * Paths are unique: an element whose path is already taken is skipped, with everything below it. The order of
	 * precedence for a name used by several kinds is the one of resolveVariable (Controllable, Measurable,
	 * Attribute).
	 *
	 * A variable object that is shared by several components (see Component::instantiate, which hands the same
	 * measurable to every instance) is listed once under every path.
	 */
	CPPDIGITALTWINLIB_EXPORT std::vector<ModelTreeEntry> walkModelTree(const std::map<std::string, Package*>& packages,
	                                                                   const std::map<std::string, Component*>& components,
	                                                                   const std::map<std::string, Port*>& ports);
}
