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
#include <sysml/occurrences/OccurrenceUsage.h>
#include <sysml/attibutes/AttributeDefinition.h>
#include <kerml/core/classifiers/Subclassification.h>
#include <sysml/metadata/MetadataUsage.h>

#include "DigitalTwinModel.h"

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

	std::string DigitalTwinModel::digitalTwinName() {
		return DigitalTwin->getName();
	}

	void DigitalTwinModel::setUpdateModelFunction(std::function<void()> updateModel) {
		UpdateModelFunction = updateModel;
	}

	void DigitalTwinModel::buildDigitalTwinModel()
	{
		const auto& textualRepresentations = Manager->downloadDigitalTwinModel(DigitalTwin->owningProject()->getId(), DigitalTwin->referencedCommit()->getId());

		std::string completeModel;

		for (const auto& elem : textualRepresentations)
			if ((std::dynamic_pointer_cast<KerML::Entities::TextualRepresentation>(elem)->language() != "Markdown") && (std::dynamic_pointer_cast<KerML::Entities::TextualRepresentation>(elem)->language() != "YaML"))
				completeModel += std::dynamic_pointer_cast<KerML::Entities::TextualRepresentation>(elem)->body();


		Instance->parseModel(completeModel);
		DigitalTwinModelElements = Instance->getElements();

		for (const auto& elem : DigitalTwinModelElements)
		{
			if (elem->qualifiedName().has_value())
			{
				std::cout << "Element name: " << elem->qualifiedName().value_or("isssue") << std::endl;
				std::cout << "Element type: " << elem->getType() << std::endl;
			}

		    //if (elem->getType()=="Package")
		    //{
		    //    std::cout << "Create package with Name: " << elem->declaredName().value_or("unnamed") << std::endl;
		    //}

		    //if (elem->getType() == "PartDefinition")
		    //{
		    //    std::cout << "Create component with Name: " << elem->declaredName().value_or("unnamed") << std::endl;
		    //}

		    //if (elem->getType() == "OccurrenceUsage")
		    //{
		    //    const auto& occurance = std::dynamic_pointer_cast<SysMLv2::Entities::OccurrenceUsage>(elem);
		    //    std::cout << "Create instance of " << /*occurance->occurrenceDefinition().front()->declaredName().value_or("issue") <<*/ "with name " << occurance->declaredName().value_or("unnamed") << std::endl;
		    //}
		}

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


		if (element->getType() == "Namespace")
		{
			for (const auto& elem : std::dynamic_pointer_cast<KerML::Entities::Namespace>(element)->ownedMember())
			{
				generateDigitalTwinModelRecursively(elem, parent);
			}
		}

		if (element->getType() == "Package")
		{
			auto newParent = new Package(element->declaredName().value());
			storePackageInModel(parent, newParent);
			for (const auto& elem : element->ownedElements())
			{
				generateDigitalTwinModelRecursively(elem, newParent);
			}
		}

		if (element->getType() == "PartDefinition")
		{
			auto newParent = new Component(element->declaredName().value());
			storeComponentInModel(parent, newParent);
			for (const auto& elem : element->ownedElements())
			{
				generateDigitalTwinModelRecursively(elem, newParent);
			}
		}

		if (element->getType() == "Function")
		{
			auto newParent = new Function(element->declaredName().value());
			storeFunctionInModel(parent, newParent);
			for (const auto& elem : element->ownedElements())
			{
				if (elem->declaredName().has_value())
				{
					if (elem->getType()=="Feature")
					{
						auto feature = std::dynamic_pointer_cast<KerML::Entities::Feature>(elem);
						newParent->appendParameter(buildVariableOfFeature(feature));
					}
				}
				else
					newParent->setReturnVariable(new RealVariable("Return"));
			}
		}


		if (element->getType() == "AttributeUsage")
		{
			auto attribute = std::dynamic_pointer_cast<SysMLv2::Entities::AttributeUsage>(element);
			auto variable = buildVariableOfDataType(attribute);
			if (variable)
			{
				ElementType type = ElementType::Variable;
				for (const auto& elem : attribute->member())
				{
					if (elem->getType()=="MetadataUsage")
					{
						type = getElementTypeOfMetaDataUsage(std::dynamic_pointer_cast<SysMLv2::Entities::MetadataUsage>(elem));
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
			}else if (attribute->ownedElements().size()>0)
			{
				 variable = buildVariableOfOwnedElements(attribute);
				if (variable)
					storeMeasurableInModel(parent, variable);
			}else
				std::cerr << "Issue with the type of the given Attribute." << std::endl;
		}

		if (element->getType() == "OccurrenceUsage")
		{
			const auto& occurrence = std::dynamic_pointer_cast<SysMLv2::Entities::OccurrenceUsage>(element);
			std::cout << "Create Instance of " << occurrence->type().front()->declaredName().value() << std::endl;
			dynamic_cast<Package*>(parent)->instantiateComponent(occurrence->declaredName().value(),occurrence->type().front()->declaredName().value());
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
		auto featureTyping = std::dynamic_pointer_cast<KerML::Entities::FeatureTyping>(attribute->ownedElements().front());
		if (featureTyping)
		{
			auto attributeDefinition = std::dynamic_pointer_cast<SysMLv2::Entities::AttributeDefinition>(featureTyping->type());
			if (attributeDefinition)
			{
				const auto subclassification = attributeDefinition->ownedSubclassification().back();
				if (subclassification->general()->declaredName().has_value())
				{
					if (subclassification->general()->declaredName().value() == "ScalarQuantityValue")
						variable = new RealVariable(attribute->declaredName().value());
				}
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

	Component* DigitalTwinModel::getComponentWithAddress(std::string address) {
		const auto splittedAdress = CPSBASELIB::STD_EXTENTION::StringExtention::splitString(address, '/');

		if (splittedAdress.size() < 1)
			throw DigitalTwinAddressException();

		if (splittedAdress.size() == 1)
			return dynamic_cast<Component*>(ComponentMap[splittedAdress[0]]);

		std::string addressWithHigherIndex = "";
		for (size_t i = 1; i < splittedAdress.size(); i++) {
			addressWithHigherIndex += splittedAdress[i];
			if (i < (splittedAdress.size() - 1))
				addressWithHigherIndex += "/";
		}

		return dynamic_cast<Component*>(ComponentMap[splittedAdress[0]])->getComponent(addressWithHigherIndex);
	}

	IVariable* DigitalTwinModel::getVariableWithAddress(std::string address) {
		const auto splittedAdress = CPSBASELIB::STD_EXTENTION::StringExtention::splitString(address, '/');

		if (splittedAdress.size() < 2)
			throw DigitalTwinAddressException();

		std::string addressWithHigherIndex = "";
		for (size_t i = 1; i < splittedAdress.size(); i++) {
			addressWithHigherIndex += splittedAdress[i];
			if (i < (splittedAdress.size() - 1))
				addressWithHigherIndex += "/";
		}

		return dynamic_cast<Component*>(ComponentMap[splittedAdress[0]])->resolveVariable(addressWithHigherIndex);
	}
}
