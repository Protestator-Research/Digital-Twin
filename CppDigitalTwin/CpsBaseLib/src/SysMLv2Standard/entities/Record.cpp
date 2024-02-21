//
// Created by Moritz Herzog on 19.02.24.
//
//---------------------------------------------------------
// Constants, Definitions, Pragmas
//---------------------------------------------------------

//---------------------------------------------------------
// External Classes
//---------------------------------------------------------
#include <boost/uuid/uuid_generators.hpp>
//---------------------------------------------------------
// Internal Classes
//---------------------------------------------------------

#include "Record.h"
namespace SysMLv2::Entities {
    Record::Record() :
            IEntity() {
        Id = boost::uuids::nil_generator()();
    }

    Record::~Record() {

    }

    Record::Record(boost::uuids::uuid id, std::list<std::string> alias, std::string name, std::string description) {
        Id=id;
        Alias = alias;
        Name = name;
        Description = description;
    }

    bool Record::operator==(const Record &other) {
        if(Id != other.Id)
            return false;
        if(Name != other.Name)
            return false;
        if(Description != other.Description)
            return false;
        if(Alias != other.Alias)
            return false;

        return true;
    }

    std::string Record::getName() const {
        return Name;
    }

    void Record::setName(std::string& name) {
        Name = name;
    }

    std::list<std::string> Record::getAlias() const {
        return Alias;
    }

    void Record::appendAlias(std::string& alias) {
        Alias.emplace_back(alias);
    }

    std::string Record::getDescription() const {
        return Description;
    }

    void Record::setDescription(std::string& description) {
        Description = description;
    }

    boost::uuids::uuid Record::getId() const {
        return Id;
    }

    std::string Record::serializeToJson() {

    }


}
