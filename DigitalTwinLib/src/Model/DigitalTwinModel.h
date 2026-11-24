//
// Created by Moritz Herzog on 23.05.24.
//

#ifndef DIGITALTWIN_DIGITALTWINMODEL_H
#define DIGITALTWIN_DIGITALTWINMODEL_H

#include <vector>
#include <string>
#include <map>
#include <any>
#include <functional>
#include <boost/uuid/uuid.hpp>

#include "../cpp_digital_twin_lib_global.h"
#include "Entities/Variables/Variable.hpp"
#include <sysmlv2/service/implementation/InstanceManager.h>

#include "Entities/ElementType.h"
#include "MQTT/entities/values/ValueEntity.h"
#include "State/StateTypes.h"

namespace SysMLv2::REST {
	class DigitalTwin;
}

namespace SysMLv2::Entities
{
	class AttributeUsage;
	class MetadataUsage;
}

namespace KerML::Entities {
	class Element;
	class NamespaceImport;
	class DataType;
	class Feature;
}

namespace DigitalTwin {
	class DigitalTwinManager;
	namespace Model {
		class Package;
		class Port;
		class IDigitalTwinElement;
		class ICollectionType;
		class Component;
		class StateTracking;
	}
	namespace Communication
	{
		class FunctionRegistrationRequest;
		class FunctionValueEntity;
	}
}

namespace DigitalTwin::Model {
	/**
	 * @class DigitalTwinModel
	 * @author Moritz Herzog
	 * @version 1.3
	 * Setup the Digital Twin model for the UI and the MQTT Backend.
	 * TODO: Split between UI and MQTT needs to be done differently.
	 */
	class CPPDIGITALTWINLIB_EXPORT DigitalTwinModel {
	public:
		DigitalTwinModel() = delete;
		explicit DigitalTwinModel(std::shared_ptr<SysMLv2::REST::DigitalTwin> digitalTwin, DigitalTwinManager* manager);
		virtual ~DigitalTwinModel();
		// The model owns the observers of its variables and raw pointers into its tree.
		DigitalTwinModel(const DigitalTwinModel&) = delete;
		DigitalTwinModel& operator=(const DigitalTwinModel&) = delete;

		/**
		 * Last topic segment of a variable that has a function attached: the computing client writes the result there.
		 * `return` is a SysML keyword, so it cannot collide with a variable or function name.
		 */
		static constexpr const char* RETURN_TOPIC = "return";

		void generateDigitalTwinBackend();

		/**
		 * Sets the variable at `path` from a JSON payload. A path that ends in RETURN_TOPIC addresses the result of
		 * a variable that has a function attached.
		 * @return The variable the call addressed first, then every other variable whose value was set during the
		 * call (linked variables). A variable object that is reachable under several paths (a measurable shared by
		 * several component instances) is reported under each of them: the path that was addressed is not derived,
		 * the others are. For a RETURN_TOPIC path the variable's own path is reported.
		 * Throws what the payload parsing or the address resolution throws; nothing is reported then.
		 */
		UpdateResult updateVariable(const std::string& path, const std::string& payload);

		/**
		 * Packages, Components, Ports and all variables of the model, parents before children. The path of a
		 * variable equals the MQTT topic path that getElementValueEntities uses (without function suffixes).
		 * See walkModelTree for the details.
		 */
		std::vector<ModelElementInfo> getModelTree();

		/** The current value of every variable of the tree (a shared variable under each of its paths). */
		std::vector<VariableState> snapshotState();

		/**
		 * Sets the values of a stored state without propagating them to linked variables. Unknown paths and values of
		 * the wrong type are skipped.
		 * @return The number of variable values applied.
		 */
		size_t restoreState(const std::vector<VariableState>& state);

		/** Commit that the twin references, the history is kept per commit. */
		boost::uuids::uuid commitId() const;
		boost::uuids::uuid projectId() const;

		std::string digitalTwinName();

		std::vector<IDigitalTwinElement*> getAllComponents() const;
		std::vector<IDigitalTwinElement*> getAllPackages() const;

		IVariable* getVariableWithAddress(std::string address);
		ICollectionType* getComponentOrPackageWithAddress(std::string address);

		std::vector<std::string> getElementStrings();
		std::vector<std::pair<std::string, std::shared_ptr<DigitalTwin::Communication::ValueEntity>>> getElementValueEntities();
		void registerFunction(DigitalTwin::Communication::FunctionRegistrationRequest* registrationRequest);

		void setUpdateModelFunction(std::function<void()> updateModel);

		boost::uuids::uuid getId() const;
		/**
		 * Creates the companion ValueEntity for the given variable, initialised with the variable's current value.
		 */
		std::shared_ptr<DigitalTwin::Communication::ValueEntity> createValueEntity(IVariable* variable);
		/**
		 * Creates the payload entity of a function topic. It holds the input parameters of the function.
		 * @return nullptr if the function is null, throws std::invalid_argument if parameter names are not unique.
		 */
		std::shared_ptr<DigitalTwin::Communication::FunctionValueEntity> createFunctionValueEntity(Function* function);

	private:
		Component* getComponentWithAddress(std::string address);
		Package* getPackageWithAddress(std::string address);
		Function* getFunctionWithAddress(std::string address);

		void buildDigitalTwinModel();

		/** Builds the path index and installs the change observer on first use. The tree is fixed after the build. */
		StateTracking& stateTracking();

		void generateDigitalTwinModelRecursively(const std::shared_ptr<KerML::Entities::Element>& element, ICollectionType* parent);
		IVariable* buildVariableOfDataType(std::shared_ptr<SysMLv2::Entities::AttributeUsage>& element);
		IVariable* buildVariableOfOwnedElements(std::shared_ptr<SysMLv2::Entities::AttributeUsage>& element);
		IVariable* buildVariableOfFeature(std::shared_ptr<KerML::Entities::Feature>& feature);
		IDigitalTwinElement* getElementFromQualifiedName(std::string qualifiedName);
		ElementType getElementTypeOfMetaDataUsage(std::shared_ptr<SysMLv2::Entities::MetadataUsage> metaDataUsage);
		DigitalTwin::Model::SupportedTypes getTypeOfSysMLType(std::shared_ptr<KerML::Entities::DataType>& type);
		DigitalTwin::Model::SupportedTypes getTypeOfSysMLType(std::string value);


		std::shared_ptr<SysMLv2::REST::DigitalTwin> DigitalTwin;
		std::vector<std::shared_ptr<KerML::Entities::Element>> DigitalTwinModelElements;
		std::vector<std::shared_ptr<KerML::Entities::Element>> RootElements;
		[[maybe_unused]] DigitalTwinManager* Manager;
		std::map<std::string, Component*> ComponentMap;
		std::map<std::string, Package*> PackageMap;
		std::map<std::string, Port*> PortMap;
		std::function<void()> UpdateModelFunction;
		SysMLv2::API::InstanceManager* Instance;
		StateTracking* Tracking = nullptr;
	};
}

#endif //DIGITALTWIN_DIGITALTWINMODEL_H
