
// Generated from ./SysMLv2.g4 by ANTLR 4.13.2


#include "SysMLv2BaseListener.h"

#include "../Model/Entities/Component.h"
#include "../Model/Entities/Package.h"
#include "../Model/Entities/Variable.h"


void SysMLv2BaseListener::enterPackage(SysMLv2Parser::PackageContext* ctx)
{
	const auto package = new DigitalTwin::Model::Package("");
	ParentStack.emplace(package);
}

void SysMLv2BaseListener::exitPackage(SysMLv2Parser::PackageContext* ctx)
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

void SysMLv2BaseListener::exitPackage_declaration(SysMLv2Parser::Package_declarationContext * ctx) 
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
	if (ctx->typed_by()==nullptr)
	{
		std::cout << "Issue while creating Attribute usage, type is needed." << std::endl;
	}
	auto type = ctx->identification().back()->NAME()->toString();
	auto name = ctx->identification().front()->NAME()->toString();
	auto variable = new DigitalTwin::Model::Variable(name,getTypeForString(type));
	if (ParentStack.size() > 0) {
		auto parentElement = dynamic_cast<DigitalTwin::Model::ICollectionType*>(ParentStack.top());
		if (ctx->usage_prefix()->getText().find("measurable") != std::string::npos)
		{
			parentElement->appendMeasurable(variable);
		}
		else if (ctx->usage_prefix()->getText().find("controllable") != std::string::npos)
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

void SysMLv2BaseListener::enterIndividual_definition(SysMLv2Parser::Individual_definitionContext* )
{

}

void SysMLv2BaseListener::exitIndividual_definition(SysMLv2Parser::Individual_definitionContext* )
{

}

void SysMLv2BaseListener::exitIndividual_usage(SysMLv2Parser::Individual_usageContext* ctx)
{

}

void SysMLv2BaseListener::enterPart_definition(SysMLv2Parser::Part_definitionContext* ctx)
{
	auto component = new DigitalTwin::Model::Component("");
	ParentStack.emplace(component);
}

void SysMLv2BaseListener::exitPart_definition(SysMLv2Parser::Part_definitionContext* ctx)
{
	auto component = dynamic_cast<DigitalTwin::Model::Component*>(ParentStack.top());
	if (component==nullptr)
	{
		std::cout << "Wrong Parent in Stack, to create component." << std::endl;
		return;
	} 
	component->setName(ctx->definition()->definition_declaration()->identification()->getText());
	ParentStack.pop();
	Elements.push_back(component);
}

void SysMLv2BaseListener::enterPart_usage(SysMLv2Parser::Part_usageContext* ctx)
{
}

void SysMLv2BaseListener::exitPart_usage(SysMLv2Parser::Part_usageContext* ctx)
{
}

void SysMLv2BaseListener::enterPort_definition(SysMLv2Parser::Port_definitionContext* ctx)
{
}

void SysMLv2BaseListener::exitPort_definition(SysMLv2Parser::Port_definitionContext* ctx)
{
}

void SysMLv2BaseListener::enterPort_usage(SysMLv2Parser::Port_usageContext* ctx)
{
}

void SysMLv2BaseListener::exitPort_usage(SysMLv2Parser::Port_usageContext* ctx)
{
}

void SysMLv2BaseListener::exitBinding_connector_as_usage(SysMLv2Parser::Binding_connector_as_usageContext* ctx)
{
	
}

std::vector<DigitalTwin::Model::IDigitalTwinElement*> SysMLv2BaseListener::getElements() const
{
	return Elements;
}

DigitalTwin::Model::SupportedTypes SysMLv2BaseListener::getTypeForString(std::string type)
{
	if ((type=="Real")||(type=="Voltage")||(type=="Current")||(type=="Resistance"))
		return DigitalTwin::Model::SupportedTypes::DOUBLE;
	
	if (type=="Integer")
		return DigitalTwin::Model::INT;
	
	if (type == "Boolean")
		return DigitalTwin::Model::BOOLEAN;
}
