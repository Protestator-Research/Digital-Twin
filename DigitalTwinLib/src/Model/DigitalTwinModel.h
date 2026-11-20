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

		/**
		 * Last topic segment of a variable that has a function attached: the computing client writes the result there.
		 * `return` is a SysML keyword, so it cannot collide with a variable or function name.
		 */
		static constexpr const char* RETURN_TOPIC = "return";

		void generateDigitalTwinBackend();

		void updateVariable(const std::string& path, const std::string& payload);

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
	};
}

#endif //DIGITALTWIN_DIGITALTWINMODEL_H
