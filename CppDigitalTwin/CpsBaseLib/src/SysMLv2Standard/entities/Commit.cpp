//
// Created by Moritz Herzog on 23.02.24.
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
#include "Commit.h"
#include "Project.h"
#include "DataVersion.h"



namespace SysMLv2::Entities{
    Commit::Commit(boost::uuids::uuid id, std::string name, std::string description, Project *owningProject,
                   Commit **previusCommits) : Record(id,name,description){
        PreviusCommits = previusCommits;
        OwningProject = owningProject;
    }

    Commit::Commit(std::string name, std::string description, Project *owningProject, Commit **previusCommits) :
            Record(boost::uuids::random_generator()(), name, description){
        OwningProject = owningProject;
        PreviusCommits = previusCommits;
    }

    Commit::Commit(std::string jsonString) : Record(jsonString) {

    }

    Commit::~Commit() {
        delete Change;
    }

    Project* Commit::getOwningProject() const {
        return OwningProject;
    }

    void Commit::setChange(DataVersion *dataVersion) {
        Change = dataVersion;
    }

    DataVersion* Commit::getDataVersion() {
        return Change;
    }

    Commit** Commit::getPreviusCommits() const {
        return PreviusCommits;
    }

    std::string Commit::serializeToJSON() {
        return std::string();
    }

}

