//
// Created by Moritz Herzog on 19.02.24.
//
//---------------------------------------------------------
// Constants, Definitions, Pragmas
//---------------------------------------------------------

//---------------------------------------------------------
// External Classes
//---------------------------------------------------------
#include <nlohmann/json.hpp>
#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
//---------------------------------------------------------
// Internal Classes
//---------------------------------------------------------
#include "Identity.h"

namespace SysMLv2::Entities {
    Identity::Identity(boost::uuids::uuid id)
    {
        Id = id;
    }

    std::string Identity::serializeToJson() {
        nlohmann::json json;

        if(!Id.is_nil()){
            json["@id"] = boost::uuids::to_string(Id);
        }

        return json.dump();
    }

    Identity::Identity() {
        Id = boost::uuids::nil_generator()();
    }

    Identity::Identity(std::string jsonString) {
        nlohmann::json JsonString = nlohmann::json::parse(jsonString);
        Id = boost::uuids::string_generator()(JsonString["@id"].get<std::string>());
    }

    boost::uuids::uuid Identity::getId() const {
        return Id;
    }

    Identity::Identity(Identity &other) {
        Id = other.Id;
    }

    Identity &Identity::operator=(const Identity &other) {
        if(this == &other)
            return *this;

        Id=other.Id;
        return *this;
    }

    Identity::Identity(Identity const &identity) {
        Id = identity.Id;
    }


}