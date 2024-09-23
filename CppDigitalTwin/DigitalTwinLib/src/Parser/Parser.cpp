//
// Created by Moritz Herzog on 09.08.24.
//

#include "Parser.h"

namespace DigitalTwin::Parser {
    std::vector<Model::Component> Parser::parse(DigitalTwin::Parser::SupportedModels models, std::string model) {
        switch (models) {
            case KerML:
                return parseKerML(model);
            case SysMLv2:
                return parseSysMLv2(model);
        }
    }

    std::vector<Model::Component> Parser::parseSysMLv2(std::string ) {

        return std::vector<Model::Component>();
    }

    std::vector<Model::Component> Parser::parseKerML(std::string ) {
        return std::vector<Model::Component>();
    }
}