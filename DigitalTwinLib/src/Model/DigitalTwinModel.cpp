//
// Created by Moritz Herzog on 23.05.24.
//

#include <sysmlv2/rest/entities/DataIdentity.h>
#include <kerml/root/elements/Element.h>
#include <BaseFuctions/StringExtention.hpp>
#include <boost/uuid/uuid.hpp>
#include <iostream>
#include <kerml/root/annotations/TextualRepresentation.h>
#include <sysmlv2/rest/entities/Commit.h>
#include <sysmlv2/rest/entities/IEntity.h>
#include <sysmlv2/rest/entities/Project.h>
#include <sysml/attributes/AttributeUsage.h>
#include <kerml/root/namespaces/NamespaceImport.h>
#include <kerml/core/types/Type.h>
#include <kerml/kernel/datatypes/DataType.h>
#include <kerml/root/namespaces/Namespace.h>
#include <kerml/kernel/expressions/FeatureReferenceExpression.h>
#include <kerml/kernel/expressions/OperatorExpression.h>
#include <sysml/occurrences/OccurrenceUsage.h>
#include <sysml/attibutes/AttributeDefinition.h>
#include <kerml/core/classifiers/Subclassification.h>
#include <sysml/metadata/MetadataUsage.h>
#include <kerml/kernel/packages/Package.h>
#include <sysml/parts/PartDefinition.h>
#include "DigitalTwinModel.h"
#include "ModelBlocks.h"
#include <cmath>
#include <stdexcept>

#include <async_mqtt/protocol/impl/buffer_to_packet_variant.ipp>
#include <async_mqtt/protocol/impl/store.hpp>

#include "Entities/IDigitalTwinElement.h"
#include "Entities/ICollectionType.h"
#include "Entities/Component.h"
#include "Entities/Package.h"
#include "Entities/Port.h"
#include "Entities/Variables/RealVariable.h"
#include "Entities/Variables/Variable.hpp"
#include "Exceptions/DigitalTwinAddressException.h"
#include "../DigitalTwinManager.h"
#include "entities/DigitalTwin.h"
#include "Entities/Function.h"

#include "Entities/Port.h"
#include "Entities/Variables/BooleanVariable.h"
#include "Entities/Variables/ComplexVariable.h"
#include "Entities/Variables/IntegerVariable.h"
#include "Entities/Variables/NaturalVariable.h"
#include "Entities/Variables/PositiveVariable.h"
#include "Entities/Variables/RationalVariable.h"
#include "Entities/Variables/StringVariable.h"
#include "Entities/Variables/ScalarQuantities.h"
#include "MQTT/entities/FunctionRegistrationRequest.hpp"
#include "MQTT/entities/values/BooleanValueEntity.h"
#include "MQTT/entities/values/ComplexValueEntity.h"
#include "MQTT/entities/values/IntegerValueEntity.h"
#include "MQTT/entities/values/PositiveValueEntity.h"
#include "MQTT/entities/values/RationalValueEntity.h"
#include "MQTT/entities/values/RealValueEntity.h"
#include "MQTT/entities/values/StringValueEntity.h"

namespace DigitalTwin::Model {

	DigitalTwinModel::DigitalTwinModel(std::shared_ptr<SysMLv2::REST::DigitalTwin> digitalTwin, DigitalTwinManager* manager) :
		DigitalTwin(digitalTwin),
		Manager(manager),
		UpdateModelFunction([] {})
	{
		Instance = new SysMLv2::API::InstanceManager();
		generateDigitalTwinBackend();
	}

	DigitalTwinModel::~DigitalTwinModel() {
		delete Instance;
	}

	void DigitalTwinModel::generateDigitalTwinBackend() {
		buildDigitalTwinModel();

	}

	void DigitalTwinModel::updateVariable(const std::string& path, const std::string& payload)
	{
		//std::cout << "Variable with Address: " << path << " is updated with Value:" << std::endl << payload << std::endl;
		const auto variableToBeEdited = getVariableWithAddress(path);
		if (variableToBeEdited!=nullptr)
		{
			//std::cout << "Variable found?" << std::endl;
			//std::cout << "Varialbe To Be Edited: " << variableToBeEdited->getName() <<std::endl;
			const std::string &type = variableToBeEdited->getType();
			if (type== "BooelanVariable")
			{
				auto json_value = DigitalTwin::Communication::BooleanValueEntity(payload);
				dynamic_cast<BooleanVariable*>(variableToBeEdited)->setValue(json_value.getValue());
			}
			if (type == "ComplexVariable")
			{
				auto json_value = DigitalTwin::Communication::ComplexValueEntity(payload);
				dynamic_cast<ComplexVariable*>(variableToBeEdited)->setValue(json_value.getValue());
			}
			if (type == "IntegerVariable")
			{
				auto json_value = DigitalTwin::Communication::IntegerValueEntity(payload);
				dynamic_cast<IntegerVariable*>(variableToBeEdited)->setValue(json_value.getValue());
			}
			if (type == "PositiveVariable")
			{
				auto json_value = DigitalTwin::Communication::PositiveValueEntity(payload);
				dynamic_cast<PositiveVariable*>(variableToBeEdited)->setValue(json_value.getValue());
			}
			if (type == "RationalVariable")
			{
				auto json_value = DigitalTwin::Communication::RationalValueEntity(payload);
				dynamic_cast<RationalVariable*>(variableToBeEdited)->setValue(json_value.getValue());
			}
			if (type == "RealVariable")
			{
				auto json_value = DigitalTwin::Communication::RealValueEntity(payload);
				dynamic_cast<RealVariable*>(variableToBeEdited)->setValue(json_value.getValue());
			}
			if (type == "StringVariable")
			{
				auto json_value = DigitalTwin::Communication::StringValueEntity(payload);
				dynamic_cast<StringVariable*>(variableToBeEdited)->setValue(json_value.getValue());
			}

			//TODO Some Variable Types are missing that need to be filled.
		}
		else
			std::cout << "Error in Variable search. Variable not found." << std::endl;
	}

	std::string DigitalTwinModel::digitalTwinName() {
		return DigitalTwin->getName();
	}

	void DigitalTwinModel::setUpdateModelFunction(std::function<void()> updateModel) {
		UpdateModelFunction = updateModel;
	}

	boost::uuids::uuid DigitalTwinModel::getId() const
	{
		return DigitalTwin->getId();
	}

	std::shared_ptr<DigitalTwin::Communication::ValueEntity> DigitalTwinModel::createValueEntity(IVariable* variable)
	{
		using namespace DigitalTwin::Communication;
		// JSON has no NaN/inf (nlohmann writes null, which the server rejects), so a non-finite initial value is 0.
		const auto finiteOrZero = [](double v) { return std::isfinite(v) ? v : 0.0; };
		if (auto real = dynamic_cast<RealVariable*>(variable))
			return std::make_shared<RealValueEntity>(finiteOrZero(real->getValue()));
		if (auto integer = dynamic_cast<IntegerVariable*>(variable))
			return std::make_shared<IntegerValueEntity>(static_cast<int>(integer->getValue()));
		if (auto boolean = dynamic_cast<BooleanVariable*>(variable))
			return std::make_shared<BooleanValueEntity>(boolean->getValue());
		if (auto complex = dynamic_cast<ComplexVariable*>(variable))
		{
			const auto value = complex->getValue();
			return std::make_shared<ComplexValueEntity>(finiteOrZero(value.real()), finiteOrZero(value.imag()));
		}
		if (auto rational = dynamic_cast<RationalVariable*>(variable))
			return std::make_shared<RationalValueEntity>(rational->getValue());
		if (auto positive = dynamic_cast<PositiveVariable*>(variable))
			return std::make_shared<PositiveValueEntity>(positive->getValue());
		if (auto string = dynamic_cast<StringVariable*>(variable))
			return std::make_shared<StringValueEntity>(string->getValue());
		if (dynamic_cast<ScalarQuantities*>(variable))
			return std::make_shared<RealValueEntity>(0.0); // Provisional default for scalar quantities.
		return std::make_shared<IntegerValueEntity>(0); // Provisional default for unknown variable types.
	}

	void DigitalTwinModel::buildDigitalTwinModel()
	{
		if (!DigitalTwin->owningProject() || !DigitalTwin->referencedCommit())
			throw std::runtime_error("The digital twin references no project or no commit, its model cannot be built.");

		const auto& elements = Manager->downloadDigitalTwinModel(DigitalTwin->owningProject()->getId(), DigitalTwin->referencedCommit()->getId());

		// Only the TextualRepresentations that contain code, in document order. Other elements (Package, memberships) are skipped.
		const std::string completeModel = assembleModelCode(elements);

		Instance->parseModel(completeModel);
		DigitalTwinModelElements = Instance->getElements();

		generateDigitalTwinModelRecursively(Instance->getRootNamespace(), nullptr);
	}

	void DigitalTwinModel::generateDigitalTwinModelRecursively(const std::shared_ptr<KerML::Entities::Element>& element, ICollectionType* parent)
	{
		std::function<void(ICollectionType*, Component*)> storeComponentInModel = [this](ICollectionType* parent, Component* comp)
			{
				if (parent == nullptr)
					ComponentMap.insert(std::make_pair(comp->getName(), comp));
				else
					parent->appendComponent(comp);
			};

		std::function<void(ICollectionType*, Port*)> storePortInModel = [this](ICollectionType* parent, Port* port)
			{
				if (parent == nullptr)
					PortMap.insert(std::make_pair(port->getName(), port));
				else
					parent->appendPort(port);
			};


		std::function<void(ICollectionType*, IVariable*)> storeAttributeInModel = [](ICollectionType* parent, IVariable* var)
			{
				parent->appendAttribute(var);
			};

		std::function<void(ICollectionType*, IVariable*)> storeMeasurableInModel = [](ICollectionType* parent, IVariable* var)
			{
				parent->appendMeasurable(var);
			};

		std::function<void(ICollectionType*, IVariable*)> storeControllableInModel = [](ICollectionType* parent, IVariable* var)
			{
				parent->appendControllable(var);
			};

		std::function<void(ICollectionType*, Package*)> storePackageInModel = [this](ICollectionType* parent, Package* pack)
			{
				if (parent == nullptr)
					PackageMap.insert(std::make_pair(pack->getName(), pack));
			};

		std::function<void(ICollectionType*, Function*)> storeFunctionInModel = [](ICollectionType* parent, Function* function)
			{
				parent->appendFunction(function);
			};

		auto namespaceRepresentation = std::dynamic_pointer_cast<KerML::Entities::Namespace>(element);
		auto package = std::dynamic_pointer_cast<KerML::Entities::Package>(element);
		auto partDefintion = std::dynamic_pointer_cast<SysMLv2::Entities::PartDefinition>(element);
		
		
		if ((namespaceRepresentation)&&(namespaceRepresentation->getType()=="Namespace"))
		{
			for (const auto& elem : namespaceRepresentation->ownedMember())
			{
				generateDigitalTwinModelRecursively(elem, parent);
			}
			return;
		}

		if ((package)&&(!partDefintion))
		{
			auto newParent = new Package(package->declaredName().value());
			storePackageInModel(parent, newParent);
			for (const auto& elem : package->ownedElements())
			{
				generateDigitalTwinModelRecursively(elem, newParent);
			}
			return;
		}

		if (partDefintion)
		{
			auto newParent = new Component(element->declaredName().value());
			storeComponentInModel(parent, newParent);
			for (const auto& elem : element->ownedElements())
			{
				generateDigitalTwinModelRecursively(elem, newParent);
			}
			return;
		}

		auto function = std::dynamic_pointer_cast<KerML::Entities::Function>(element);
		if (function)
		{
			auto newParent = new Function(element->declaredName().value());
			storeFunctionInModel(parent, newParent);
			for (const auto& elem : element->ownedElements())
			{
				if (elem->declaredName().has_value())
				{
					if (elem->getType() == "Feature")
					{
						auto feature = std::dynamic_pointer_cast<KerML::Entities::Feature>(elem);
						newParent->appendParameter(buildVariableOfFeature(feature));
					}
				}
				else
					newParent->setReturnVariable(new RealVariable("Return"));
			}
			return;
		}

		auto attribute = std::dynamic_pointer_cast<SysMLv2::Entities::AttributeUsage>(element);
		if (attribute)
		{
			auto variable = buildVariableOfDataType(attribute);

			if ((variable == nullptr) && (attribute->ownedElements().size() > 0))
			{
				variable = buildVariableOfOwnedElements(attribute);
				if (variable)
					storeMeasurableInModel(parent, variable);
			}

			if (variable) {
				ElementType type = ElementType::Variable;
				for (const auto& elem : attribute->member())
				{
					auto metadataUsage = std::dynamic_pointer_cast<SysMLv2::Entities::MetadataUsage>(elem);
					if (metadataUsage)
					{
						type = getElementTypeOfMetaDataUsage(metadataUsage);
					}
					const auto operatorExpression = std::dynamic_pointer_cast<KerML::Entities::OperatorExpression > (elem);
					if (operatorExpression)
					{
						
					}
					const auto invocationExpression = std::dynamic_pointer_cast<KerML::Entities::InvocationExpression>(elem);
					if ((invocationExpression) && (!operatorExpression))
					{
						if (invocationExpression->instantiatedType()->qualifiedName().has_value()) {
							const auto qualifiedName = invocationExpression->instantiatedType()->qualifiedName().value();
							const auto invokedFunction = dynamic_cast<Function*>(getElementFromQualifiedName(qualifiedName))->invocate();
							if (invokedFunction) {
								for (const auto& arg : invocationExpression->argument())
								{
									const auto reference = std::dynamic_pointer_cast<KerML::Entities::FeatureReferenceExpression>(arg);
									if (reference->referent()->qualifiedName().has_value()) {
										const auto connectedVariable = dynamic_cast<IVariable*>(getElementFromQualifiedName(reference->referent()->qualifiedName().value()));
										invokedFunction->appendParameter(connectedVariable);
									}
								}
								invokedFunction->setReturnVariable(variable);
								variable->setFunctionInvocation(invokedFunction);
							}
						}
					}

				}
				switch (type)
				{
				case Measurable:
					storeMeasurableInModel(parent, variable);
					break;
				case Controllable:
					storeControllableInModel(parent, variable);
					break;
				case Constant:
				case Variable:
					storeAttributeInModel(parent, variable);
					break;
				default:
					break;
				}
			}
			else
				std::cerr << "Issue with the type of the given Attribute." << std::endl;

			return;
		}

		const auto& occurrence = std::dynamic_pointer_cast<SysMLv2::Entities::OccurrenceUsage>(element);
		if (occurrence)
		{
			dynamic_cast<Package*>(parent)->instantiateComponent(occurrence->declaredName().value(), occurrence->type().front()->declaredName().value());
			return;
		}
	}

	IVariable* DigitalTwinModel::buildVariableOfDataType(std::shared_ptr<SysMLv2::Entities::AttributeUsage>& attribute)
	{
		IVariable* variable = nullptr;
		if (!attribute->attributeDefinition().empty())
		{
			switch (getTypeOfSysMLType(attribute->attributeDefinition().front()))
			{
			case BOOLEAN:
				variable = new BooleanVariable(attribute->declaredName().value());
				break;
			case STRING:
				variable = new StringVariable(attribute->declaredName().value());
				break;
			case COMPLEX:
				variable = new ComplexVariable(attribute->declaredName().value());
				break;
			case REAL:
				variable = new RealVariable(attribute->declaredName().value());
				break;
			case RATIONAL:
				variable = new RationalVariable(attribute->declaredName().value());
				break;
			case INTEGER:
				variable = new IntegerVariable(attribute->declaredName().value());
				break;
			case NATURAL:
				//variable = new NaturalVariable(attribute->declaredName().value());
				variable = new IntegerVariable(attribute->declaredName().value());
				break;
			case POSITIVE:
				variable = new PositiveVariable(attribute->declaredName().value());
				break;
			case NA:
			default:
				break;
			}
		}
		return variable;
	}

	IVariable* DigitalTwinModel::buildVariableOfOwnedElements(std::shared_ptr<SysMLv2::Entities::AttributeUsage>& attribute)
	{
		IVariable* variable = nullptr;
		auto attributeDefinition = std::dynamic_pointer_cast<SysMLv2::Entities::AttributeDefinition>(attribute->attributeDefinition().back());
		if (attributeDefinition)
		{
			const auto subclassification = attributeDefinition->ownedSubclassification().back();
			if (subclassification->general()->declaredName().has_value())
			{
				if (subclassification->general()->declaredName().value() == "ScalarQuantityValue")
					variable = new RealVariable(attribute->declaredName().value());
			}
		}
		return variable;
	}

	IVariable* DigitalTwinModel::buildVariableOfFeature(std::shared_ptr<KerML::Entities::Feature>& feature)
	{
		const auto featureType = feature->type().back();
		if (featureType)
		{
			IVariable* variable = nullptr;
			if (!featureType->declaredName().has_value())
			{
				switch (getTypeOfSysMLType(featureType->declaredName().value()))
				{
				case BOOLEAN:
					variable = new BooleanVariable(feature->declaredName().value());
					break;
				case STRING:
					variable = new StringVariable(feature->declaredName().value());
					break;
				case COMPLEX:
					variable = new ComplexVariable(feature->declaredName().value());
					break;
				case REAL:
					variable = new RealVariable(feature->declaredName().value());
					break;
				case RATIONAL:
					variable = new RationalVariable(feature->declaredName().value());
					break;
				case INTEGER:
					variable = new IntegerVariable(feature->declaredName().value());
					break;
				case NATURAL:
					//variable = new NaturalVariable(feature->declaredName().value());
					variable = new IntegerVariable(feature->declaredName().value());
					break;
				case POSITIVE:
					variable = new PositiveVariable(feature->declaredName().value());
					break;
				case NA:
				default:
					break;
				}
			}
			return variable;
		}
		return nullptr;
	}

	IDigitalTwinElement* DigitalTwinModel::getElementFromQualifiedName(std::string qualifiedName)
	{
		std::string address = CPSBASELIB::STD_EXTENTION::StringExtention::replaceAll(qualifiedName, "::", "/");
		IDigitalTwinElement* element = nullptr;

		try
		{
			element=getFunctionWithAddress(address);
		}catch (...)
		{
			element=nullptr;
		}

		if (!element)
		{
			try
			{
				element=getPackageWithAddress(address);
			}catch (...)
			{
				element=nullptr;
			}
		}
		if (!element)
		{
			try
			{
				element=getVariableWithAddress(address);
			}catch (...)
			{
				element=nullptr;
			}
		}
		if (!element)
		{
			try
			{
				element = getComponentOrPackageWithAddress(address);
			}catch (...)
			{
				element=nullptr;
			}
		}

		return element;
	}

	ElementType DigitalTwinModel::getElementTypeOfMetaDataUsage(
		std::shared_ptr<SysMLv2::Entities::MetadataUsage> metaDataUsage)
	{
		if (metaDataUsage)
		{
			if (metaDataUsage->occurrenceDefinition().front())
			{
				if (metaDataUsage->occurrenceDefinition().front()->declaredName() == "Measurable")
				{
					return Measurable;
				}
				if (metaDataUsage->occurrenceDefinition().front()->declaredName() == "Controllable")
				{
					return Controllable;
				}
			}
		}
		return Variable;
	}

	DigitalTwin::Model::SupportedTypes DigitalTwinModel::getTypeOfSysMLType(
		std::shared_ptr<KerML::Entities::DataType>& type)
	{
		return getTypeOfSysMLType(type->qualifiedName().value());
	}

	DigitalTwin::Model::SupportedTypes DigitalTwinModel::getTypeOfSysMLType(std::string value)
	{
		if (value == "ScalarValues::Real")
			return SupportedTypes::REAL;
		if (value == "ScalarValues::Boolean")
			return SupportedTypes::BOOLEAN;
		if (value == "ScalarValues::String")
			return SupportedTypes::STRING;
		if (value == "ScalarValues::Integer")
			return SupportedTypes::INTEGER;
		if (value == "ScalarValues::Complex")
			return SupportedTypes::COMPLEX;
		if (value == "ScalarValues::Rational")
			return SupportedTypes::RATIONAL;
		if (value == "ScalarValues::Natural")
			return SupportedTypes::NATURAL;
		if (value == "ScalarValues::Positive")
			return SupportedTypes::POSITIVE;
		return SupportedTypes::NA;
	}

	std::vector<IDigitalTwinElement*> DigitalTwinModel::getAllComponents() const {
		std::vector<IDigitalTwinElement*> returnValue = std::vector<IDigitalTwinElement*>();

		for (auto element : ComponentMap)
			returnValue.push_back(element.second);

		return returnValue;
	}

	std::vector<IDigitalTwinElement*> DigitalTwinModel::getAllPackages() const
	{
		std::vector<IDigitalTwinElement*> returnValue = std::vector<IDigitalTwinElement*>();

		for (auto element : PackageMap)
			returnValue.push_back(element.second);

		return returnValue;
	}

	std::vector<std::string> DigitalTwinModel::getElementStrings() {
		std::vector<std::string> elements;

		for (const auto& element : PackageMap)
			for (const auto& string : dynamic_cast<Package*>(element.second)->getAllMQTTTopics())
				elements.push_back(element.first + "/" + string);


		for (const auto& element : ComponentMap)
			for (const auto& string : dynamic_cast<Component*>(element.second)->getAllMQTTTopics())
				elements.push_back(element.first + "/" + string);

		return elements;
	}

	std::vector<std::pair<std::string, std::shared_ptr<DigitalTwin::Communication::ValueEntity>>> DigitalTwinModel::getElementValueEntities() {
		std::vector<std::pair<std::string, std::shared_ptr<DigitalTwin::Communication::ValueEntity>>> elements;

		for (const auto& element : PackageMap) {
			const auto package = dynamic_cast<Package*>(element.second);
			if (!package)
				continue;
			for (const auto& variable : package->getAllMQTTVariables())
				elements.push_back(std::make_pair(element.first + "/" + variable.first, createValueEntity(variable.second)));
		}

		for (const auto& element : ComponentMap) {
			const auto component = dynamic_cast<Component*>(element.second);
			if (!component)
				continue;
			for (const auto& variable : component->getAllMQTTVariables())
				elements.push_back(std::make_pair(element.first + "/" + variable.first, createValueEntity(variable.second)));
		}

		return elements;
	}

	void DigitalTwinModel::registerFunction(DigitalTwin::Communication::FunctionRegistrationRequest* functionRegistrationRequest)
	{
		std::string functionName = functionRegistrationRequest->getQualifiedName();
		std::cout<<"Registering Function: "<<functionName<<std::endl;
		std::string functionPartsInDTCode = CPSBASELIB::STD_EXTENTION::StringExtention::replaceAll(functionName, "::", "/");

		const auto parts = CPSBASELIB::STD_EXTENTION::StringExtention::splitString(functionPartsInDTCode, '/');

		std::string componentPath = "";
		for (size_t i = 0; i < parts.size() - 1; i++)
		{
			componentPath += parts[i];
			if (i < (parts.size() - 2))
				componentPath += "/";
		}

		const auto element = getComponentOrPackageWithAddress(componentPath);
		if (element)
			std::cout<<"Registration Element: "<<element->getName()<<std::endl;

	}

	Component* DigitalTwinModel::getComponentWithAddress(std::string address) {
		const auto splittedAdress = CPSBASELIB::STD_EXTENTION::StringExtention::splitString(address, '/');

		if (splittedAdress.size() < 1)
			throw DigitalTwinAddressException();

		if (splittedAdress.size() == 1)
		{
			if (ComponentMap.contains(splittedAdress[0]))
				return dynamic_cast<Component*>(ComponentMap[splittedAdress[0]]);
			else
				return nullptr;
		}

		std::string addressWithHigherIndex = "";
		for (size_t i = 1; i < splittedAdress.size(); i++) {
			addressWithHigherIndex += splittedAdress[i];
			if (i < (splittedAdress.size() - 1))
				addressWithHigherIndex += "/";
		}
		if (ComponentMap.contains(splittedAdress[0]))
			return dynamic_cast<Component*>(ComponentMap[splittedAdress[0]])->getComponent(addressWithHigherIndex);

		return nullptr;
	}

	Package* DigitalTwinModel::getPackageWithAddress(std::string address)
	{
		const auto splittedAdress = CPSBASELIB::STD_EXTENTION::StringExtention::splitString(address, '/');

		if (splittedAdress.size() < 1)
			throw DigitalTwinAddressException();

		if (splittedAdress.size() == 1)
			return dynamic_cast<Package*>(PackageMap[splittedAdress[0]]);

		// std::string addressWithHigherIndex = "";
		// for (size_t i = 1; i < splittedAdress.size(); i++) {
		// 	addressWithHigherIndex += splittedAdress[i];
		// 	if (i < (splittedAdress.size() - 1))
		// 		addressWithHigherIndex += "/";
		// }
		//
		// return dynamic_cast<Component*>(PackageMap[splittedAdress[0]])->getComponent(addressWithHigherIndex);
		return nullptr;
	}

	Function* DigitalTwinModel::getFunctionWithAddress(std::string address)
	{
		auto parts = CPSBASELIB::STD_EXTENTION::StringExtention::splitString(address, '/');
		std::string componentAddress = "";
		for (size_t i = 0; i<parts.size()-1; i++)
		{
			componentAddress += parts[i];
			if (i < parts.size()-2)
			{
				componentAddress+="/";
			}
		}
		const auto element = getComponentOrPackageWithAddress(componentAddress);
		if (element)
		{
			const auto function = element->resolveFunction(parts.back());
			if (function)
				return function;
		}
		return nullptr;
	}

	IVariable* DigitalTwinModel::getVariableWithAddress(std::string address) {
		const auto splittedAdress = CPSBASELIB::STD_EXTENTION::StringExtention::splitString(address, '/');
		//std::cout << "Address Splitted" << std::endl;

		if (splittedAdress.size() < 2)
			throw DigitalTwinAddressException();

		//std::cout << "Address is bigger than 2" << std::endl;

		std::string addressWithHigherIndex = "";
		for (size_t i = 1; i < splittedAdress.size(); i++) {
			addressWithHigherIndex += splittedAdress[i];
			if (i < (splittedAdress.size() - 1))
				addressWithHigherIndex += "/";
		}

		//std::cout << "Addresses Spllitted into:" << std::endl << "Current Interation index: " << splittedAdress[0] << std::endl << "Higher Index" << addressWithHigherIndex <<std::endl;

		if (ComponentMap.contains(splittedAdress[0])) {
			//std::cout << "Checking in Components " << std::endl;
			return dynamic_cast<Component*>(ComponentMap[splittedAdress[0]])->resolveVariable(addressWithHigherIndex);
		}
		else {
			//std::cout << "Checking in Packages " << std::endl;
			return dynamic_cast<Package*>(PackageMap[splittedAdress[0]])->resolveVariable(addressWithHigherIndex);
		}
	}

	ICollectionType* DigitalTwinModel::getComponentOrPackageWithAddress(std::string address)
	{
		ICollectionType* returnValue = getComponentWithAddress(address);
		if (!returnValue)
			returnValue = getPackageWithAddress(address);

		return returnValue;
	}
}
