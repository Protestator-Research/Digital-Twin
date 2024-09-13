//
// Created by Moritz Herzog on 09.08.24.
//

#ifndef DIGITALTWIN_PARSER_H
#define DIGITALTWIN_PARSER_H

#include <string>
#include <vector>

#include "../Model/Component.h"
#include "SupportedModels.h"

namespace DigitalTwin::Parser {
    class Parser {
    public:
        //Parser();
        //virtual ~Parser();

        static std::vector<Model::Component> parse(SupportedModels models, std::string model);

    private:
        static std::vector<Model::Component> parseSysMLv2(std::string model);
        static std::vector<Model::Component> parseKerML(std::string model);
    };
}


#endif //DIGITALTWIN_PARSER_H
