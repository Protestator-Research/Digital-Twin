#include "ModelBlocks.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <nlohmann/json.hpp>

#include <kerml/root/annotations/TextualRepresentation.h>
#include <kerml/root/elements/Element.h>
#include <sysmlv2/rest/entities/CommitRequest.h>
#include <sysmlv2/rest/entities/DataVersion.h>

namespace DigitalTwin::Model {
    namespace {
        using nlohmann::json;

        std::string referenceId(const json& reference) {
            if (reference.is_object()) {
                auto id = reference.find("@id");
                if (id != reference.end() && id->is_string())
                    return id->get<std::string>();
            }
            return "";
        }

        /// Ids of a reference array, in the given order.
        std::vector<std::string> referenceIds(const json& value) {
            std::vector<std::string> result;
            if (value.is_array())
                for (const auto& item : value)
                    if (auto id = referenceId(item); !id.empty())
                        result.push_back(id);
            return result;
        }

        json parseElement(const std::shared_ptr<KerML::Entities::Element>& element) {
            if (!element)
                return json();
            try {
                return json::parse(element->serializeToJson(), nullptr, false);
            } catch (...) {
                return json();
            }
        }

        void report(std::vector<std::string>* warnings, const std::string& message) {
            if (warnings)
                warnings->push_back(message);
            else
                std::cerr << "[DigitalTwinLib] " << message << std::endl;
        }

        /// The ids of the blocks in the order of the root package, empty if there is no usable root package.
        std::vector<std::string> documentOrder(const std::vector<std::shared_ptr<KerML::Entities::Element>>& elements,
                                               std::vector<std::string>* warnings) {
            std::map<std::string, json> byId;
            std::vector<json> rootPackages;
            for (const auto& element : elements) {
                json parsed = parseElement(element);
                if (!parsed.is_object())
                    continue;
                const std::string id = parsed.value("@id", std::string());
                if (id.empty())
                    continue;
                // A root has no owner at all: neither owner nor owningRelationship nor owningRelatedElement.
                if (parsed.value("@type", std::string()) == "Package" &&
                    referenceId(parsed.value("owner", json())).empty() &&
                    referenceId(parsed.value("owningRelationship", json())).empty() &&
                    referenceId(parsed.value("owningRelatedElement", json())).empty())
                    rootPackages.push_back(parsed);
                byId.emplace(id, std::move(parsed));
            }

            if (rootPackages.empty()) {
                report(warnings, "The model has no root package, the order of the blocks is not known. "
                                 "The order given by the server (sorted by UUID) is used.");
                return {};
            }
            std::sort(rootPackages.begin(), rootPackages.end(), [](const json& a, const json& b) {
                return a.value("@id", std::string()) < b.value("@id", std::string());
            });
            if (rootPackages.size() > 1)
                report(warnings, "The model has " + std::to_string(rootPackages.size()) +
                                 " root packages, the first one by UUID is used for the order of the blocks.");

            std::vector<std::string> order;
            for (const auto& membershipId : referenceIds(rootPackages.front().value("ownedRelationship", json()))) {
                auto membership = byId.find(membershipId);
                if (membership == byId.end())
                    continue;
                auto owned = referenceIds(membership->second.value("ownedRelatedElement", json()));
                if (owned.empty()) {
                    // fallback: the member element of the membership
                    if (auto member = referenceId(membership->second.value("memberElement", json())); !member.empty())
                        owned.push_back(member);
                }
                order.insert(order.end(), owned.begin(), owned.end());
            }
            return order;
        }
    }

    std::vector<std::shared_ptr<KerML::Entities::TextualRepresentation>>
    codeBlocksInOrder(const std::vector<std::shared_ptr<KerML::Entities::Element>>& elements,
                      std::vector<std::string>* warnings) {
        std::vector<std::shared_ptr<KerML::Entities::TextualRepresentation>> blocks;
        for (const auto& element : elements)
            if (auto block = std::dynamic_pointer_cast<KerML::Entities::TextualRepresentation>(element))
                blocks.push_back(block);

        // Sort by the position in the root package. The sort is stable, unreferenced blocks keep their given order and go last.
        const auto order = documentOrder(elements, warnings);
        if (!order.empty()) {
            std::map<std::string, std::size_t> position;
            for (std::size_t i = 0; i < order.size(); ++i)
                position.emplace(order[i], i);
            auto positionOf = [&position](const std::shared_ptr<KerML::Entities::TextualRepresentation>& block) {
                auto found = position.find(boost::uuids::to_string(block->getId()));
                return found == position.end() ? std::string::npos : found->second;
            };
            std::stable_sort(blocks.begin(), blocks.end(), [&](const auto& a, const auto& b) {
                return positionOf(a) < positionOf(b);
            });
            if (std::any_of(blocks.begin(), blocks.end(), [&](const auto& b) { return positionOf(b) == std::string::npos; }))
                report(warnings, "Some blocks are not referenced by the root package and are appended at the end.");
        }

        std::vector<std::shared_ptr<KerML::Entities::TextualRepresentation>> result;
        for (const auto& block : blocks)
            if (!block->body().empty() && block->language() != "Markdown" && block->language() != "YaML" &&
                block->language() != "HTML")
                result.push_back(block);
        return result;
    }

    std::string assembleModelCode(const std::vector<std::shared_ptr<KerML::Entities::Element>>& elements,
                                  std::vector<std::string>* warnings) {
        std::string code;
        for (const auto& block : codeBlocksInOrder(elements, warnings))
            code += block->body();
        return code;
    }

    DocumentCommit createDocumentCommit(const std::string& packageName,
                                        const std::vector<std::shared_ptr<KerML::Entities::Element>>& blocks,
                                        const std::string& description) {
        if (blocks.empty())
            throw std::invalid_argument("The document has no content, nothing can be committed.");

        boost::uuids::random_generator newId;
        const std::string packageId = boost::uuids::to_string(newId());

        DocumentCommit result;
        json ownedRelationship = json::array();
        std::vector<std::shared_ptr<KerML::Entities::Element>> memberElements;

        for (const auto& block : blocks) {
            json blockJson = parseElement(block);
            if (!blockJson.is_object() || blockJson.value("@id", std::string()).empty())
                throw std::invalid_argument("A block of the document cannot be serialized.");

            const std::string blockId = blockJson["@id"].get<std::string>();
            const std::string membershipId = boost::uuids::to_string(newId());

            blockJson["owner"] = {{"@id", packageId}};
            blockJson["owningRelationship"] = {{"@id", membershipId}};

            json membership = {
                {"@type", "OwningMembership"},
                {"@id", membershipId},
                {"elementId", membershipId},
                {"source", json::array({{{"@id", packageId}}})},
                {"target", json::array({{{"@id", blockId}}})},
                {"owningRelatedElement", {{"@id", packageId}}},
                {"ownedRelatedElement", json::array({{{"@id", blockId}}})},
                {"memberElement", {{"@id", blockId}}}
            };
            ownedRelationship.push_back({{"@id", membershipId}});
            memberElements.push_back(KerML::Entities::Element::fromJson(membership.dump()));
            memberElements.push_back(KerML::Entities::Element::fromJson(blockJson.dump()));
        }

        json package = {
            {"@type", "Package"},
            {"@id", packageId},
            {"elementId", packageId},
            {"declaredName", packageName},
            {"ownedRelationship", ownedRelationship}
        };
        result.Elements.push_back(KerML::Entities::Element::fromJson(package.dump()));
        result.Elements.insert(result.Elements.end(), memberElements.begin(), memberElements.end());

        std::vector<std::shared_ptr<SysMLv2::REST::DataVersion>> changes;
        changes.reserve(result.Elements.size());
        for (const auto& element : result.Elements)
            changes.push_back(std::make_shared<SysMLv2::REST::DataVersion>(element));
        result.Request = std::make_shared<SysMLv2::REST::CommitRequest>(description, changes);
        return result;
    }
}
