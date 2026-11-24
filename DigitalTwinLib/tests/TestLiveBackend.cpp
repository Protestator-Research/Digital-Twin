// Live integration test against a running SysML v2 API server (StructuraSystemsServer). It is skipped unless
// DT_TEST_SERVER_URL is set. Further variables: DT_TEST_USER and DT_TEST_PASSWORD (default: admin / empty).
// Use a test database only. Example: DT_TEST_SERVER_URL=http://127.0.0.1:18090 DT_TEST_USER=admin DT_TEST_PASSWORD=...
#include <gtest/gtest.h>

#include <chrono>
#include <cstdlib>
#include <string>
#include <vector>

#include <boost/uuid/uuid_io.hpp>

#include <kerml/root/annotations/TextualRepresentation.h>
#include <kerml/root/elements/Element.h>
#include <sysmlv2/rest/entities/Branch.h>
#include <sysmlv2/rest/entities/Commit.h>
#include <sysmlv2/rest/entities/Project.h>
#include <sysmlv2/rest/entities/CommitRequest.h>
#include <sysmlv2/rest/entities/DataVersion.h>

#include "BECommunicationService.h"
#include "entities/DigitalTwin.h"
#include "entities/TwinRequest.h"
#include "Model/ModelBlocks.h"

using BACKEND_COMMUNICATION::CommunicationService;
using KerML::Entities::Element;
using KerML::Entities::TextualRepresentation;

namespace {
    std::string env(const char* name, const char* fallback = "") {
        const char* value = std::getenv(name);
        return value ? value : fallback;
    }

    class LiveBackend : public ::testing::Test {
    protected:
        void SetUp() override {
            const std::string url = env("DT_TEST_SERVER_URL");
            if (url.empty())
                GTEST_SKIP() << "DT_TEST_SERVER_URL is not set, the live test is skipped.";
            Service = std::make_unique<CommunicationService>(url, "");
            ASSERT_TRUE(Service->setUserForLoginInBackend(env("DT_TEST_USER", "admin"), env("DT_TEST_PASSWORD")));
            Suffix = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        }

        std::vector<std::shared_ptr<Element>> numberedBlocks(int codeBlocks) {
            std::vector<std::shared_ptr<Element>> blocks;
            blocks.push_back(std::make_shared<TextualRepresentation>("Markdown", "# Document " + Suffix));
            for (int i = 0; i < codeBlocks; ++i) {
                blocks.push_back(std::make_shared<TextualRepresentation>("SysMLv2", "part def P" + std::to_string(i) + ";\n"));
                if (i % 3 == 0)
                    blocks.push_back(std::make_shared<TextualRepresentation>("Markdown", "text " + std::to_string(i)));
            }
            blocks.push_back(std::make_shared<TextualRepresentation>("YaML", "key: value"));
            return blocks;
        }

        static std::vector<std::string> expectedCode(const std::vector<std::shared_ptr<Element>>& blocks) {
            std::vector<std::string> result;
            for (const auto& block : blocks)
                if (auto tr = std::dynamic_pointer_cast<TextualRepresentation>(block))
                    if (tr->language() == "SysMLv2")
                        result.push_back(tr->body());
            return result;
        }

        std::shared_ptr<SysMLv2::REST::Commit> commitDocument(const std::shared_ptr<SysMLv2::REST::Project>& project,
                                                             const std::vector<std::shared_ptr<Element>>& blocks) {
            auto documentCommit = DigitalTwin::Model::createDocumentCommit(project->getName(), blocks);
            auto commit = Service->postCommitWithId(project->getId(), documentCommit.Request);
            EXPECT_NE(commit, nullptr);
            return commit;
        }

        std::unique_ptr<CommunicationService> Service;
        std::string Suffix;
    };
}

TEST_F(LiveBackend, NewProjectHasNoHeadAndOrderSurvivesTheServer) {
    auto project = Service->postProject("dt-live-" + Suffix, "live test", "main");
    ASSERT_NE(project, nullptr);

    // a new project has no commit: the default branch has no head
    auto branches = Service->getAllBranchesForProjectWithID(project->getId());
    ASSERT_EQ(branches.size(), 1u);
    EXPECT_EQ(branches[0]->getName(), "main");
    EXPECT_EQ(branches[0]->getHead(), nullptr);

    // first commit: root package, memberships and blocks in document order
    auto blocks = numberedBlocks(25);
    auto commit = commitDocument(project, blocks);
    ASSERT_NE(commit, nullptr);

    branches = Service->getAllBranchesForProjectWithID(project->getId());
    ASSERT_EQ(branches.size(), 1u);
    ASSERT_NE(branches[0]->getHead(), nullptr);
    EXPECT_EQ(branches[0]->getHead()->getId(), commit->getId());

    auto elements = Service->getAllElementsOfCommit(project->getId(), commit->getId());
    EXPECT_EQ(elements.size(), 1u + 2u * blocks.size());

    std::vector<std::string> warnings;
    auto code = DigitalTwin::Model::codeBlocksInOrder(elements, &warnings);
    EXPECT_TRUE(warnings.empty()) << (warnings.empty() ? "" : warnings[0]);
    std::vector<std::string> bodies;
    for (const auto& block : code)
        bodies.push_back(block->body());
    EXPECT_EQ(bodies, expectedCode(blocks));

    // the digital twin can be created and listed
    auto twinRequest = std::make_shared<SysMLv2::REST::TwinRequest>("twin-" + Suffix, commit);
    auto twin = Service->postDigitalTwin(twinRequest, project->getId());
    ASSERT_NE(twin, nullptr);
    bool found = false;
    for (const auto& listed : Service->getAllDigitalTwinsForProjectWithId(project->getId()))
        found = found || (listed->getId() == twin->getId());
    EXPECT_TRUE(found);
}

TEST_F(LiveBackend, MoreThanOnePageOfElementsArrivesCompletely) {
    auto project = Service->postProject("dt-live-big-" + Suffix, "live test", "main");
    ASSERT_NE(project, nullptr);

    auto blocks = numberedBlocks(130); // far more than 100 elements: the server paginates
    auto commit = commitDocument(project, blocks);
    ASSERT_NE(commit, nullptr);

    auto elements = Service->getAllElementsOfCommit(project->getId(), commit->getId());
    EXPECT_GT(elements.size(), 100u);
    EXPECT_EQ(elements.size(), 1u + 2u * blocks.size());

    std::vector<std::string> bodies;
    for (const auto& block : DigitalTwin::Model::codeBlocksInOrder(elements))
        bodies.push_back(block->body());
    EXPECT_EQ(bodies, expectedCode(blocks));
}

TEST_F(LiveBackend, ListsOfProjectsAreComplete) {
    // more projects than one page (100): none of them may be cut off
    const std::size_t before = Service->getAllProjects().size();
    const std::size_t toCreate = before < 105 ? 105 - before : 1;
    for (std::size_t i = 0; i < toCreate; ++i)
        ASSERT_NE(Service->postProject("dt-live-list-" + Suffix + "-" + std::to_string(i), "", "main"), nullptr);
    EXPECT_EQ(Service->getAllProjects().size(), before + toCreate);
}

TEST_F(LiveBackend, EmptyCommitIsRejectedAndDescribed) {
    auto project = Service->postProject("dt-live-empty-" + Suffix, "", "main");
    ASSERT_NE(project, nullptr);
    try {
        Service->postCommitWithId(project->getId(), std::make_shared<SysMLv2::REST::CommitRequest>("empty", std::vector<std::shared_ptr<SysMLv2::REST::DataVersion>>{}));
        FAIL() << "an empty commit must be rejected";
    } catch (const std::exception& ex) {
        EXPECT_NE(CommunicationService::describeBackendError(ex).find("rejected as invalid"), std::string::npos)
            << CommunicationService::describeBackendError(ex);
    }
}
