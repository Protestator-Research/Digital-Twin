//
// Created by Moritz Herzog on 05.09.24.
//
//---------------------------------------------------------
// Constants, Definitions, Pragmas
//---------------------------------------------------------

//---------------------------------------------------------
// External Classes
//---------------------------------------------------------
#include <nlohmann/json.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>
//---------------------------------------------------------
// Internal Classes
//---------------------------------------------------------
#include "Identification.h"
#include "JSONEntities.h"


namespace SysMLv2::Entities {

    Identification::Identification(boost::uuids::uuid id) {
        Id = id;
    }

    Identification::Identification(std::string JSONstring) {
        nlohmann::json json = nlohmann::json::parse(JSONstring);
        Id = boost::uuids::string_generator()(json[JSON_ID_ENTITY].get<std::string>());
    }

    std::string Identification::serializeToJson() {
        nlohmann::json json;
        json[JSON_ID_ENTITY] = boost::lexical_cast<std::string>(Id);
        return json.dump();
    }

    boost::uuids::uuid Identification::getID() {
        return Id;
    }
}
