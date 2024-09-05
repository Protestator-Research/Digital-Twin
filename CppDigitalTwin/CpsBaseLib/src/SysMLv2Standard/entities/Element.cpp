//
// Created by Moritz Herzog on 27.02.24.
//

#include <nlohmann/json.hpp>

#include "Element.h"

namespace SysMLv2 {
    namespace Entities {
        Element::Element() : Data() {

        }

        Element::Element(std::string jsonString) : Data(jsonString) {
            nlohmann::json json = nlohmann::json::parse(jsonString);
        }

        std::string Element::serializeToJson() {
            return Data::serializeToJson();
        }

        Element::~Element() {

        }
    } // Entities
} // SysMLv2