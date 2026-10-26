#include <gtest/gtest.h>

#include <algorithm>
#include <random>
#include <string>
#include <vector>

#include <boost/uuid/uuid_io.hpp>
#include <nlohmann/json.hpp>

#include <kerml/root/annotations/TextualRepresentation.h>
#include <kerml/root/elements/Element.h>
#include <sysmlv2/rest/entities/CommitRequest.h>
#include <sysmlv2/rest/entities/DataVersion.h>

#include <sysmlv2/rest/serialization/SysMLv2Deserializer.h>

#include "Model/ModelBlocks.h"

using namespace DigitalTwin::Model;
using KerML::Entities::Element;
using KerML::Entities::TextualRepresentation;
using nlohmann::json;

namespace {
    std::shared_ptr<TextualRepresentation> block(const std::string& language, const std::string& body) {
        return std::make_shared<TextualRepresentation>(language, body);
    }

    std::vector<std::shared_ptr<Element>> asElements(const std::vector<std::shared_ptr<TextualRepresentation>>& blocks) {
        return {blocks.begin(), blocks.end()};
    }

    std::vector<std::string> bodies(const std::vector<std::shared_ptr<TextualRepresentation>>& blocks) {
        std::vector<std::string> result;
        for (const auto& b : blocks)
            result.push_back(b->body());
        return result;
    }

    /// What a client receives for a document: the elements of createDocumentCommit as the server stores them, read back
    /// by the typed deserializer of the library (TextualRepresentation, Package, ...), sorted by UUID like the server does.
    /// The original block objects are NOT part of it (their ids are the ones of the received blocks).
    std::vector<std::shared_ptr<Element>> serverView(const std::vector<std::shared_ptr<TextualRepresentation>>& blocks,
                                                     bool withPackage = true) {
        auto commit = createDocumentCommit("Doc", asElements(blocks));
        json array = json::array();
        for (const auto& element : commit.Elements)
            array.push_back(json::parse(element->serializeToJson()));
        std::vector<std::shared_ptr<Element>> elements;
        for (const auto& entity : SysMLv2::SysMLv2Deserializer::deserializeJsonArray(array.dump())) {
            auto element = std::dynamic_pointer_cast<Element>(entity);
            if (!element)
                continue;
            if (!withPackage && !std::dynamic_pointer_cast<TextualRepresentation>(element))
                continue;
            elements.push_back(element);
        }
        std::sort(elements.begin(), elements.end(), [](const auto& a, const auto& b) { return a->getId() < b->getId(); });
        return elements;
    }
}

TEST(ModelBlocks, ElementsOfCreateDocumentCommitAreNotTypedBlocks) {
    // Documents why the client must reload the elements from the server after a commit: the created elements are plain
    // Elements, only the deserialized ones are TextualRepresentations.
    std::vector<std::shared_ptr<TextualRepresentation>> blocks = {block("SysMLv2", "part a;")};
    auto commit = createDocumentCommit("Doc", asElements(blocks));
    EXPECT_TRUE(codeBlocksInOrder(commit.Elements).empty());
    EXPECT_EQ(bodies(codeBlocksInOrder(serverView(blocks))), (std::vector<std::string>{"part a;"}));
}

TEST(ModelBlocks, OrderComesFromTheRootPackageNotFromTheUuid) {
    // Many blocks, so that the UUID order differs from the document order with certainty.
    std::vector<std::shared_ptr<TextualRepresentation>> blocks;
    for (int i = 0; i < 40; ++i)
        blocks.push_back(block("SysMLv2", "part p" + std::to_string(i) + ";"));
    std::vector<std::string> expected = bodies(blocks);

    auto sorted = serverView(blocks);
    std::vector<std::shared_ptr<TextualRepresentation>> uuidOrder;
    for (const auto& e : sorted)
        if (auto b = std::dynamic_pointer_cast<TextualRepresentation>(e))
            uuidOrder.push_back(b);
    ASSERT_NE(bodies(uuidOrder), expected);

    std::vector<std::string> warnings;
    EXPECT_EQ(bodies(codeBlocksInOrder(sorted, &warnings)), expected);
    EXPECT_TRUE(warnings.empty());

    // also for any other input order
    std::mt19937 random(42);
    std::shuffle(sorted.begin(), sorted.end(), random);
    EXPECT_EQ(bodies(codeBlocksInOrder(sorted, &warnings)), expected);
}

TEST(ModelBlocks, MarkdownYamlAndEmptyBlocksAreSkippedButKeepTheirPlace) {
    std::vector<std::shared_ptr<TextualRepresentation>> blocks = {
        block("Markdown", "# Title"), block("SysMLv2", "package A;"), block("YaML", "a: 1"),
        block("SysMLv2", ""), block("Markdown", "text"), block("SysMLv2", "package B;")};
    std::vector<std::string> warnings;
    EXPECT_EQ(bodies(codeBlocksInOrder(serverView(blocks), &warnings)), (std::vector<std::string>{"package A;", "package B;"}));
    EXPECT_TRUE(warnings.empty());
}

TEST(ModelBlocks, OtherElementTypesAndNullPointersAreSkipped) {
    std::vector<std::shared_ptr<TextualRepresentation>> blocks = {block("SysMLv2", "a"), block("SysMLv2", "b")};
    auto elements = serverView(blocks);
    elements.push_back(nullptr);
    elements.push_back(Element::fromJson(R"({"@type":"PartDefinition","@id":"6e701bf8-7793-49de-9b0e-2f43376edabf"})"));
    EXPECT_EQ(bodies(codeBlocksInOrder(elements)), (std::vector<std::string>{"a", "b"}));
    EXPECT_TRUE(codeBlocksInOrder({}).empty());
}

TEST(ModelBlocks, WithoutPackageTheGivenOrderIsKeptWithAWarning) {
    std::vector<std::shared_ptr<TextualRepresentation>> blocks = {block("SysMLv2", "x"), block("SysMLv2", "y"), block("SysMLv2", "z")};
    std::vector<std::string> warnings;
    EXPECT_EQ(bodies(codeBlocksInOrder(asElements(blocks), &warnings)), (std::vector<std::string>{"x", "y", "z"}));
    ASSERT_EQ(warnings.size(), 1u);
    EXPECT_NE(warnings[0].find("no root package"), std::string::npos);
}

TEST(ModelBlocks, SeveralRootPackagesUseTheFirstByUuidWithAWarning) {
    std::vector<std::shared_ptr<TextualRepresentation>> first = {block("SysMLv2", "a1"), block("SysMLv2", "a2")};
    std::vector<std::shared_ptr<TextualRepresentation>> second = {block("SysMLv2", "b1"), block("SysMLv2", "b2")};
    auto viewA = serverView(first);
    auto viewB = serverView(second);
    // the package of a view is the Element of type Package
    auto packageId = [](const std::vector<std::shared_ptr<Element>>& view) {
        for (const auto& e : view)
            if (e->getType() == "Package")
                return boost::uuids::to_string(e->getId());
        return std::string();
    };
    const bool aFirst = packageId(viewA) < packageId(viewB);

    auto all = viewA;
    all.insert(all.end(), viewB.begin(), viewB.end());
    std::vector<std::string> warnings;
    auto result = bodies(codeBlocksInOrder(all, &warnings));
    ASSERT_FALSE(warnings.empty());
    EXPECT_NE(warnings[0].find("2 root packages"), std::string::npos);
    // the blocks of the chosen package come first, in their order; the others are appended
    const std::vector<std::string> expected = aFirst ? std::vector<std::string>{"a1", "a2", "b1", "b2"}
                                                      : std::vector<std::string>{"b1", "b2", "a1", "a2"};
    EXPECT_EQ(result.size(), 4u);
    EXPECT_EQ(std::vector<std::string>(result.begin(), result.begin() + 2),
              std::vector<std::string>(expected.begin(), expected.begin() + 2));
}

TEST(ModelBlocks, BlocksNotReferencedByThePackageGoLast) {
    std::vector<std::shared_ptr<TextualRepresentation>> blocks = {block("SysMLv2", "p1"), block("SysMLv2", "p2")};
    auto elements = serverView(blocks);
    elements.push_back(block("SysMLv2", "stray"));
    std::vector<std::string> warnings;
    EXPECT_EQ(bodies(codeBlocksInOrder(elements, &warnings)), (std::vector<std::string>{"p1", "p2", "stray"}));
    EXPECT_FALSE(warnings.empty());
}

TEST(ModelBlocks, DocumentCommitHasPackageMembershipsAndLinkedBlocks) {
    std::vector<std::shared_ptr<Element>> blocks = {block("Markdown", "# T"), block("SysMLv2", "part a;"), block("YaML", "k: v")};
    auto commit = createDocumentCommit("MyProject", blocks, "first");

    ASSERT_EQ(commit.Elements.size(), 1u + 2u * blocks.size());
    EXPECT_EQ(commit.Request->description(), "first");
    EXPECT_EQ(commit.Request->changeRequested().size(), commit.Elements.size());

    json package = json::parse(commit.Elements[0]->serializeToJson());
    EXPECT_EQ(package["@type"], "Package");
    EXPECT_EQ(package["declaredName"], "MyProject");
    EXPECT_TRUE(!package.contains("owner") || package["owner"].is_null());
    ASSERT_EQ(package["ownedRelationship"].size(), blocks.size());

    for (std::size_t i = 0; i < blocks.size(); ++i) {
        json membership = json::parse(commit.Elements[1 + 2 * i]->serializeToJson());
        json child = json::parse(commit.Elements[2 + 2 * i]->serializeToJson());
        EXPECT_EQ(membership["@type"], "OwningMembership");
        EXPECT_EQ(membership["@id"], package["ownedRelationship"][i]["@id"]);
        EXPECT_EQ(membership["owningRelatedElement"]["@id"], package["@id"]);
        EXPECT_EQ(membership["ownedRelatedElement"][0]["@id"], child["@id"]);
        EXPECT_EQ(child["@id"], boost::uuids::to_string(blocks[i]->getId()));
        EXPECT_EQ(child["owner"]["@id"], package["@id"]);
        EXPECT_EQ(child["owningRelationship"]["@id"], membership["@id"]);
        EXPECT_EQ(child["@type"], "TextualRepresentation");
    }
    EXPECT_EQ(json::parse(commit.Elements[2]->serializeToJson())["body"], "# T");
    EXPECT_EQ(json::parse(commit.Elements[2]->serializeToJson())["language"], "Markdown");

    // the request is valid JSON with one change per element
    json request = json::parse(commit.Request->serializeToJson());
    EXPECT_EQ(request["change"].size(), commit.Elements.size());

    // the given objects stay unchanged
    EXPECT_FALSE(json::parse(blocks[0]->serializeToJson()).contains("owningRelationship") &&
                 !json::parse(blocks[0]->serializeToJson())["owningRelationship"].is_null());
}

TEST(ModelBlocks, EmptyDocumentCannotBeCommitted) {
    EXPECT_THROW(createDocumentCommit("Empty", {}), std::invalid_argument);
}

TEST(ModelBlocks, HtmlBlocksAreNoCodeButEmptyLanguageIs) {
    std::vector<std::shared_ptr<TextualRepresentation>> blocks = {
        block("HTML", "<div>x</div>"), block("", "fence without info"), block("SysMLv2", "part a;")};
    EXPECT_EQ(bodies(codeBlocksInOrder(serverView(blocks))), (std::vector<std::string>{"fence without info", "part a;"}));
}

TEST(ModelBlocks, PackageWithAnOwningRelationshipIsNoRoot) {
    std::vector<std::shared_ptr<TextualRepresentation>> blocks = {block("SysMLv2", "a"), block("SysMLv2", "b")};
    auto elements = serverView(blocks);
    // A nested package: owned by a membership, but without an owner entry. It must not be taken as a second root.
    elements.push_back(Element::fromJson(R"({"@type":"Package","@id":"00000000-0000-4000-8000-000000000001",
        "owningRelationship":{"@id":"00000000-0000-4000-8000-000000000002"}})"));
    elements.push_back(Element::fromJson(R"({"@type":"Package","@id":"00000000-0000-4000-8000-000000000003",
        "owningRelatedElement":{"@id":"00000000-0000-4000-8000-000000000004"}})"));
    std::vector<std::string> warnings;
    EXPECT_EQ(bodies(codeBlocksInOrder(elements, &warnings)), (std::vector<std::string>{"a", "b"}));
    EXPECT_TRUE(warnings.empty());
}

TEST(ModelBlocks, AssembledCodeFollowsTheDocumentOrder) {
    std::vector<std::shared_ptr<TextualRepresentation>> blocks;
    std::string expected;
    for (int i = 0; i < 30; ++i) {
        blocks.push_back(block("SysMLv2", "part q" + std::to_string(i) + ";\n"));
        blocks.push_back(block("Markdown", "text"));
        expected += "part q" + std::to_string(i) + ";\n";
    }
    EXPECT_EQ(assembleModelCode(serverView(blocks)), expected);
}
