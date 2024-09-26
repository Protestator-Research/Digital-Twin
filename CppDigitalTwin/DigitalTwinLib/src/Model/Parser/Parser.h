//
// Created by Moritz Herzog on 09.08.24.
//

#ifndef DIGITALTWIN_PARSER_H
#define DIGITALTWIN_PARSER_H

#include <string>
#include <vector>

#include "SupportedModels.h"
#include "SysMLv2/SysMLv2Parser.h"
#include "../SupportedTypes/SupportedTypes.h"

namespace DigitalTwin::Model {
    class IDigitalTwinElement;
    class Component;
    class Port;
    class Connection;
}

namespace DigitalTwin::Parser {
    class Parser {
    public:
        //Parser();
        //virtual ~Parser();

        static std::vector<Model::IDigitalTwinElement*> parse(SupportedModels models, std::string& model);

    private:
        static std::vector<Model::IDigitalTwinElement*> parseSysMLv2(std::string& model);

        static std::vector<Model::IDigitalTwinElement*> generateComponentVector(SysMLv2Parser::StartContext* context);

        static std::vector<Model::IDigitalTwinElement*> parseKerML(std::string& model);

        static Model::SupportedTypes getTypeForTypeString(std::string& type);

        static Model::Component* generatePart(SysMLv2Parser::PartContext* context);
        static Model::Port* generatePort(SysMLv2Parser::PortContext* context);
        static Model::Connection* generateConnection(SysMLv2Parser::ConnectToContext* context);

        static Model::Component* addAttributeToPart(Model::Component* component, SysMLv2Parser::AttributeContext* context);

        static Model::IDigitalTwinElement* getElementWithNameInVector(std::string name, std::vector<Model::IDigitalTwinElement*> elements);

        static std::vector<Model::IDigitalTwinElement*> Elements;

        static std::string getNameOfString(std::vector<antlr4::tree::TerminalNode*> nameElements);
    };
}


#endif //DIGITALTWIN_PARSER_H
