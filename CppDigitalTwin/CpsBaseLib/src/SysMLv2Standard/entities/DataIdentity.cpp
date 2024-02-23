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
#include "DataIdentity.h"
#include "JSONEntities.h"

namespace SysMLv2::Entities {
    DataIdentity::DataIdentity(boost::uuids::uuid id)
    {
        Id = id;
    }

    std::string DataIdentity::serializeToJson() {
        nlohmann::json json;

        if(!Id.is_nil()){
            json[JSON_ID_ENTITY] = boost::uuids::to_string(Id);
        }

        return json.dump(JSON_INTENT);
    }

    DataIdentity::DataIdentity() {
        Id = boost::uuids::nil_generator()();
    }

    DataIdentity::DataIdentity(std::string jsonString) {
        nlohmann::json JsonString = nlohmann::json::parse(jsonString);
        Id = boost::uuids::string_generator()(JsonString[JSON_ID_ENTITY].get<std::string>());
    }

    boost::uuids::uuid DataIdentity::getId() const {
        return Id;
    }

    DataIdentity::DataIdentity(DataIdentity &other) {
        Id = other.Id;
    }

    DataIdentity &DataIdentity::operator=(const DataIdentity &other) {
        if(this == &other)
            return *this;

        Id=other.Id;
        return *this;
    }

    DataIdentity::DataIdentity(DataIdentity const &identity) {
        Id = identity.Id;
    }

    bool DataIdentity::operator==(const DataIdentity &other) {
        return Id == other.Id;
    }
}