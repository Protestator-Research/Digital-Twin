//
// Created by Moritz Herzog on 25.02.26.
//

#include "TwinRequest.h"

#include <stdexcept>
#include <nlohmann/json.hpp>
#include <sysmlv2/rest/entities/JSONEntities.h>

namespace SysMLv2::REST {
    TwinRequest::TwinRequest(std::string name, std::shared_ptr<SysMLv2::REST::Commit> referencedCommit) : IEntity() {
        Type = "TwinRequest";
        Name = name;
        ReferencedCommit = referencedCommit;
    }

    TwinRequest::TwinRequest(std::string jsonString) {
        Type = "TwinRequest";
        deserializeAndPopulate(jsonString);
    }

    std::string TwinRequest::serializeToJson() {
        if (!ReferencedCommit)
            throw std::invalid_argument("A digital twin needs a referenced commit. The project has no commit yet.");
        nlohmann::json json;
        json[SysMLv2::REST::JSON_TYPE_ENTITY] = Type;
        json[SysMLv2::REST::JSON_NAME_ENTITY] = Name;
        json[SysMLv2::REST::JSON_REFERENCE_COMMIT] = nlohmann::json::parse(ReferencedCommit->serializeIdentification());
        return json.dump(JSON_INTENT);
    }

    std::string TwinRequest::name() {
        return Name;
    }

    std::shared_ptr<SysMLv2::REST::Commit> TwinRequest::referencedCommit() {
        return ReferencedCommit;
    }

    void TwinRequest::deserializeAndPopulate(const std::string &jsonString) {
        nlohmann::json json = nlohmann::json::parse(jsonString);
        Name = json[SysMLv2::REST::JSON_NAME_ENTITY];
        ReferencedCommit = std::make_shared<SysMLv2::REST::Commit>(json[SysMLv2::REST::JSON_REFERENCE_COMMIT].dump());
    }
} // SysMLv2::REST