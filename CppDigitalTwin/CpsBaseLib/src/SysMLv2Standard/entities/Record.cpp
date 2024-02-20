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


}
