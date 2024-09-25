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
        std::vector<Model::IDigitalTwinElement*> returnValue;

        SysMLv2BaseListener listener;

        antlr4::tree::ParseTreeWalker walker;
        walker.walk(&listener,context);

        std::cout << "Get Depth of ParseTree: " << context->depth() << std::endl;
        auto elements = context->elemements();
        std::cout << "Number of Elements to be parsed: " << elements.size() << std::endl;

        for (const auto & element : elements) {
            if(element->port() != nullptr)
            {
                const auto &port = element->port();
                Model::Port* _port = new Model::Port(port->NAME()->toString());
                returnValue.push_back(_port);
            }
            if(element->part() != nullptr){
                const auto &part = element->part();
                Model::Component* _part = new Model::Component(part->NAME()->toString());
                returnValue.push_back(_part);

                if(part->delimiter_rule()->DELIMITER() != nullptr)
                    throw new std::exception();

                auto partElements = part->delimiter_rule()->bracketed_content()->elemements();
                for(auto intElement : partElements) {
                    if(intElement->attribute() != nullptr){

                    }
                    if(intElement->port() != nullptr){

                    }
                    if(intElement->part() != nullptr){

                    }
                }
            }
            if(element->connectTo()!= nullptr){
                const auto &connection = element->connectTo();
                Model::Connection* _connection = new Model::Connection("");
                returnValue.push_back(_connection);
            }
        }


        return returnValue;
    }
}