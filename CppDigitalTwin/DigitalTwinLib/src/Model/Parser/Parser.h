//
// Created by Moritz Herzog on 09.08.24.
//

#ifndef DIGITALTWIN_PARSER_H
#define DIGITALTWIN_PARSER_H

#include <string>
#include <vector>

#include "SupportedModels.h"
#include "SysMLv2/SysMLv2Parser.h"

namespace DigitalTwin::Model {
    class IDigitalTwinElement;
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

    };
}


#endif //DIGITALTWIN_PARSER_H
