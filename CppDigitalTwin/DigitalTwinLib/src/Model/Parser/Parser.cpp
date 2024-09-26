//
// Created by Moritz Herzog on 09.08.24.
//

#include "Parser.h"
#include <antlr4-runtime/antlr4-runtime.h>
#include <antlr4-runtime/antlr4-common.h>
#include "SysMLv2/SysMLv2Lexer.h"
#include "SysMLv2/SysMLv2Parser.h"
#include "SysMLv2/SysMLv2Listener.h"
#include "SysMLv2/SysMLv2BaseListener.h"
#include "../Entities/Port.h"
#include "../Entities/IDigitalTwinElement.h"
#include "../Entities/Component.h"
#include "../Entities/Connection.h"
#include "../Entities/Variable.h"

namespace DigitalTwin::Parser {
    std::vector<Model::IDigitalTwinElement*> DigitalTwin::Parser::Parser::Elements = std::vector<Model::IDigitalTwinElement*>();

    std::vector<Model::IDigitalTwinElement*> Parser::parse(DigitalTwin::Parser::SupportedModels models, std::string& model) {
        switch (models) {
            case KerML:
                return parseKerML(model);
            case SysMLv2:
                return parseSysMLv2(model);
        }
    }

    std::vector<Model::IDigitalTwinElement*> Parser::parseSysMLv2(std::string& expression) {
        std::cout << "Expression: "<< std::endl << expression << std::endl;
        antlr4::ANTLRInputStream input(expression);
        SysMLv2Lexer lexer(&input);

        antlr4::CommonTokenStream tokens(&lexer);

        SysMLv2Parser parser(&tokens);

        SysMLv2Parser::StartContext *startContext = nullptr;

        try {
            // parse the input
            startContext = parser.start();
        }
        catch (antlr4::ParseCancellationException &e) {
            std::cout << e.what() << std::endl;
            return std::vector<Model::IDigitalTwinElement*>();
        }

        std::cout << "Number of Parse Errors: "<<parser.getNumberOfSyntaxErrors() << std::endl;

        return generateComponentVector(startContext);
    }

    std::vector<Model::IDigitalTwinElement*> Parser::parseKerML(std::string& ) {
        return std::vector<Model::IDigitalTwinElement*>();
    }

    std::vector<Model::IDigitalTwinElement*> Parser::generateComponentVector(SysMLv2Parser::StartContext *context) {
        Elements = std::vector<Model::IDigitalTwinElement*>();

        SysMLv2BaseListener listener;

        antlr4::tree::ParseTreeWalker walker;
        walker.walk(&listener,context);

        std::cout << "Get Depth of ParseTree: " << context->depth() << std::endl;
        auto elements = context->elemements();
        std::cout << "Number of Elements to be parsed: " << elements.size() << std::endl;

        for (const auto & element : elements) {

            if(element->port() != nullptr)
                Elements.push_back(generatePort(element->port()));

            if(element->part() != nullptr)
                Elements.push_back(generatePart(element->part()));

            if(element->connectTo()!= nullptr)
                Elements.push_back(generateConnection(element->connectTo()));


        }

        return Elements;
    }

    Model::SupportedTypes Parser::getTypeForTypeString(std::string &type) {

        if(type.compare("Voltage") == 0)
            return Model::SupportedTypes::DOUBLE;
        if(type.compare("Double") == 0)
            return Model::SupportedTypes::DOUBLE;
        if(type.compare("Current") == 0)
            return Model::SupportedTypes::DOUBLE;
        if(type.compare("Resistance") == 0)
            return Model::SupportedTypes::DOUBLE;
        if(type.compare("Boolean") == 0)
            return Model::SupportedTypes::BOOLEAN;

        return Model::SupportedTypes::NA;
    }

    Model::Component *Parser::generatePart(SysMLv2Parser::PartContext *context) {
        Model::Component* returnValue = new Model::Component(context->NAME()->toString());

        if(context->delimiter_rule()->DELIMITER() != nullptr) {
            auto elementNames = context->specilization()->address()->NAME();
            std::string name = getNameOfString(elementNames);
            auto element = getElementWithNameInVector(name,Elements);

            if(element == nullptr)
                throw new std::exception();

            return dynamic_cast<Model::Component*>(element);
        }else {
            auto partElements = context->delimiter_rule()->bracketed_content()->elemements();
            for (auto intElement: partElements) {
                if (intElement->attribute() != nullptr)
                    addAttributeToPart(returnValue, intElement->attribute());

                if (intElement->part() != nullptr) {
                    auto partToBeAdded = generatePart(intElement->part());
                    returnValue->appendComponent(partToBeAdded);
                }

//                if (intElement->port() != nullptr) {
//                    auto portToBeAdded = generatePort(intElement->port());

//                }
            }
        }
        return returnValue;
    }

    Model::Connection *Parser::generateConnection(SysMLv2Parser::ConnectToContext *) {
        Model::Connection* connection = new Model::Connection("");
        return connection;
    }

    Model::Port *Parser::generatePort(SysMLv2Parser::PortContext *context) {
        Model::Port* port = new Model::Port(context->NAME()->toString());
        return port;
    }

    Model::Component* Parser::addAttributeToPart(Model::Component *component, SysMLv2Parser::AttributeContext *context) {
        const auto addressData = context->type_definition()->address()->NAME();
        std::string addressString = getNameOfString(addressData);
         auto attribute = new Model::Variable(context->NAME()->toString(),getTypeForTypeString(addressString));


        if(context->MEASURABLE() != nullptr) {
            component->appendMeasurable(attribute);
        }
        if(context->CONTROLLABLE() != nullptr) {
            component->appendControllable(attribute);
        }
        if((context->ATTRIBUTE() != nullptr)||(context->VARIABLE() != nullptr)) {
            component->appendAttribute(attribute);
        }

        return component;
    }

    Model::IDigitalTwinElement* Parser::getElementWithNameInVector(std::string name, std::vector<Model::IDigitalTwinElement *> elements) {
        for(auto element : elements)
            if(element->getName()==name)
                return element;


        return nullptr;
    }

    std::string Parser::getNameOfString(std::vector<antlr4::tree::TerminalNode *> nameElements) {
        std::string addressString;

        for (size_t i = 0; i<nameElements.size(); i++) {
            addressString += nameElements.at(i)->toString();
            if(i<nameElements.size()-1)
                addressString+="::";
        }
        return addressString;
    }
}