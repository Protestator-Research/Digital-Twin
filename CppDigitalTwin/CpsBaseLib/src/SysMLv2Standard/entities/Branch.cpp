//
// Created by Moritz Herzog on 23.02.24.
//

#include "Branch.h"

namespace SysMLv2::Entities {
    Branch::Branch(std::string jsonStringOrName) : CommitReference(jsonStringOrName) {

    }

    bool Branch::operator==(Branch &other) {
        return (CommitReference)(*this)==other;
    }

    std::string Branch::serializeToJson() {
        return CommitReference::serializeToJson();
    }

    Commit* Branch::getHead()
    {
        return Head;
    }
}
