
// Generated from ./SysMLv2.g4 by ANTLR 4.13.2


#include "SysMLv2BaseListener.h"

#include <any>

#include "../Model/DigitalTwinModel.h"
#include "../Model/Entities/Component.h"
#include "../Model/Entities/Package.h"
#include "../Model/Entities/Port.h"
#include "../Model/Entities/Variables/Variable.hpp"
#include "BaseFuctions/StringExtention.hpp"


void SysMLv2BaseListener::enterPackage(SysMLv2Parser::PackageContext* )
{
	const auto package = new DigitalTwin::Model::Package("");
	ParentStack.emplace(package);
}

void SysMLv2BaseListener::exitPackage(SysMLv2Parser::PackageContext* )
{
	const auto package = dynamic_cast<DigitalTwin::Model::Package*>(ParentStack.top());
	if (package == nullptr)
	{
		std::cout << "Issue while casting into Package during Parsing" << std::endl;
		return;
	}
	ParentStack.pop();
	Elements.push_back(package);
}

void SysMLv2BaseListener::exitPackage_declaration(SysMLv2Parser::Package_declarationContext* ctx)
{
	const auto package = dynamic_cast<DigitalTwin::Model::Package*>(ParentStack.top());
	if (package == nullptr)
	{
		std::cout << "Issue while casting into Package during Parsing" << std::endl;
		return;
	}
	package->setName(ctx->identification()->getText());
}

void SysMLv2BaseListener::exitAttribute_usage(SysMLv2Parser::Attribute_usageContext* ctx)
{
	if (ctx->typed_by() == nullptr)
	{
		std::cout << "Issue while creating Attribute usage, type is needed." << std::endl;
	}
	auto type = ctx->identification().back()->NAME()->toString();
	auto name = ctx->identification().front()->NAME()->toString();
	auto variable = new DigitalTwin::Model::Variable<std::any>(name);

	if (ParentStack.size() > 0) {
		auto parentElement = dynamic_cast<DigitalTwin::Model::ICollectionType*>(ParentStack.top());
		std::string usage_prefix_text = ctx->usage_prefix()->getText();
		if (usage_prefix_text.find("measurable") != std::string::npos)
		{
			parentElement->appendMeasurable(variable);
		}
		else if (usage_prefix_text.find("controllable") != std::string::npos)
		{
			parentElement->appendControllable(variable);
		}
		else
		{
			parentElement->appendAttribute(variable);
		}
		Elements.push_back(variable);
	}
}

void SysMLv2BaseListener::enterIndividual_definition(SysMLv2Parser::Individual_definitionContext*)
{

}

void SysMLv2BaseListener::exitIndividual_definition(SysMLv2Parser::Individual_definitionContext*)
{

}

void SysMLv2BaseListener::exitIndividual_usage(SysMLv2Parser::Individual_usageContext* ctx)
{

	const auto memberName = ctx->usage()->usage_declaration()->identification()->getText();
	const auto partDefinition = ctx->usage()->usage_declaration()->feature_specialization_part()->feature_specilization().front()->typings()->owned_typed_by()->owned_feature_typing()->general_type()->getText();
	const auto elemToInit = getElementWithName(partDefinition);
	if (elemToInit != nullptr)
	{
		const auto compToInit = dynamic_cast<DigitalTwin::Model::Component*>(elemToInit);
		if (compToInit != nullptr)
		{
			//TODO Create Individual Type.
			const auto individual_init = compToInit->instantiate(memberName);
			Elements.push_back(individual_init);
		}
	}
}

void SysMLv2BaseListener::enterPart_definition(SysMLv2Parser::Part_definitionContext*)
{
	auto component = new DigitalTwin::Model::Component("");
	ParentStack.emplace(component);
}

void SysMLv2BaseListener::exitPart_definition(SysMLv2Parser::Part_definitionContext* ctx)
{
	auto component = dynamic_cast<DigitalTwin::Model::Component*>(ParentStack.top());
	if (component == nullptr)
	{
		std::cout << "Wrong Parent in Stack, to create component." << std::endl;
		return;
	}
	component->setName(ctx->definition()->definition_declaration()->identification()->getText());
	ParentStack.pop();
	Elements.push_back(component);
}

void SysMLv2BaseListener::exitPart_usage(SysMLv2Parser::Part_usageContext* ctx)
{
	if (ParentStack.size() > 0)
	{
		const auto memberName = ctx->usage()->usage_declaration()->identification()->getText();
		const auto partDefinition = ctx->usage()->usage_declaration()->feature_specialization_part()->feature_specilization().front()->typings()->owned_typed_by()->owned_feature_typing()->general_type()->getText();
		const auto elemToInit = getElementWithName(partDefinition);
		const auto parentElement = dynamic_cast<DigitalTwin::Model::ICollectionType*>(ParentStack.top());
		if (elemToInit != nullptr)
		{
			const auto compToInit = dynamic_cast<DigitalTwin::Model::Component*>(elemToInit);
			if ((compToInit != nullptr) && (parentElement != nullptr))
			{
				parentElement->appendComponent(compToInit->instantiate(memberName));
			}
		}
	}
}

void SysMLv2BaseListener::enterPort_definition(SysMLv2Parser::Port_definitionContext*)
{
	auto port = new DigitalTwin::Model::Port("");
	ParentStack.emplace(port);
}

void SysMLv2BaseListener::exitPort_definition(SysMLv2Parser::Port_definitionContext* ctx)
{
	auto port = dynamic_cast<DigitalTwin::Model::Port*>(ParentStack.top());
	if (port == nullptr)
	{
		std::cout << "Wrong Parent in Stack, to create component." << std::endl;
		return;
	}
	port->setName(ctx->definition()->definition_declaration()->identification()->getText());
	ParentStack.pop();
	Elements.push_back(port);
}

void SysMLv2BaseListener::exitPort_usage(SysMLv2Parser::Port_usageContext* ctx)
{
	if (ParentStack.size() > 0)
	{
		const auto memberName = ctx->usage()->usage_declaration()->identification()->getText();
		const auto portDefinition = ctx->usage()->usage_declaration()->feature_specialization_part()->feature_specilization().front()->typings()->owned_typed_by()->owned_feature_typing()->general_type()->getText();
		const auto elemToInit = getElementWithName(portDefinition);
		const auto parentElement = dynamic_cast<DigitalTwin::Model::ICollectionType*>(ParentStack.top());
		if (elemToInit != nullptr)
		{
			const auto portToInstaciate = dynamic_cast<DigitalTwin::Model::Port*>(elemToInit);
			if ((portToInstaciate != nullptr) && (parentElement != nullptr))
			{
				parentElement->appendPort(portToInstaciate);
			}
		}
	}
}

void SysMLv2BaseListener::exitBinding_connector_as_usage(SysMLv2Parser::Binding_connector_as_usageContext* ctx)
{
	const auto firstVariableName = ctx->connector_end_member().front()->getText();
	const auto secondVariableName = ctx->connector_end_member().front()->getText();


	if (ParentStack.size() == 0) {
		const auto firstElement = resolveElement(firstVariableName);
		const auto secondElement = resolveElement(secondVariableName);

		if ((firstElement != nullptr) && (secondElement != nullptr))
		{
			const auto firstVariable = dynamic_cast<DigitalTwin::Model::Variable<std::any>*>(firstElement);
			const auto secondVariable = dynamic_cast<DigitalTwin::Model::Variable<std::any>*>(secondElement);

			if ((firstVariable != nullptr) && (secondVariable != nullptr))
			{
				firstVariable->addLinkToVariable(secondVariable);
				secondVariable->addLinkToVariable(firstVariable);
			}
		}
	} else
	{
		const auto firstVariable = dynamic_cast<DigitalTwin::Model::ICollectionType*>(ParentStack.top())->resolveVariable(firstVariableName);
		const auto secondVariable = dynamic_cast<DigitalTwin::Model::ICollectionType*>(ParentStack.top())->resolveVariable(secondVariableName);
		if ((firstVariable != nullptr) && (secondVariable != nullptr))
		{
			firstVariable->addLinkToVariable(secondVariable);
			secondVariable->addLinkToVariable(firstVariable);
		}
	}
}

std::vector<DigitalTwin::Model::IDigitalTwinElement*> SysMLv2BaseListener::getElements() const
{
	return Elements;
}

DigitalTwin::Model::IDigitalTwinElement* SysMLv2BaseListener::resolveElement(std::string connectorEndMember)
{
	const auto splittedAdress = CPSBASELIB::STD_EXTENTION::StringExtention::splitString(connectorEndMember, '.');
	
	if (splittedAdress.size() == 1)
		return getElementWithName(splittedAdress.front());
	else
		return dynamic_cast<DigitalTwin::Model::ICollectionType*>(getElementWithName(splittedAdress.front()))->resolveVariable(splittedAdress, 1);
}

DigitalTwin::Model::SupportedTypes SysMLv2BaseListener::getTypeForString(std::string type)
{
	if ((type == "Real") || (type == "Voltage") || (type == "Current") || (type == "Resistance"))
		return DigitalTwin::Model::SupportedTypes::DOUBLE;

	if (type == "Integer")
		return DigitalTwin::Model::INT;

	if (type == "Boolean")
		return DigitalTwin::Model::BOOLEAN;

	return DigitalTwin::Model::NA;
}

DigitalTwin::Model::IDigitalTwinElement* SysMLv2BaseListener::getElementWithName(std::string name)
{
	for (const auto& elem : Elements)
		if (elem->getName() == name)
			return elem;
	return nullptr;
}
