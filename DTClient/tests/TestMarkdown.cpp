// Tests of the MarkdownParser (no GUI) and, if DT_TEST_SERVER_URL is set, the upload of the real Markdown files of
// the repository to a SysML v2 API server and the assembly of the model code from the downloaded commit.
#include <gtest/gtest.h>

#include <cstdlib>
#include <chrono>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <QString>

#include <kerml/root/annotations/TextualRepresentation.h>
#include <kerml/root/elements/Element.h>
#include <sysmlv2/rest/entities/Commit.h>
#include <sysmlv2/rest/entities/Project.h>

#include "MarkdownParser.h"
#include "BECommunicationService.h"
#include "Model/ModelBlocks.h"

using DigitalTwin::Client::MarkdownParser;
using KerML::Entities::TextualRepresentation;

namespace {
    struct Block {
        std::string Language;
        std::string Body;
    };

    std::vector<Block> parse(const QString& markdown) {
        MarkdownParser parser;
        parser.parseMarkdown(markdown);
        std::vector<Block> result;
        for (const auto& element : parser.getElementsOfProject()) {
            auto tr = std::dynamic_pointer_cast<TextualRepresentation>(element);
            result.push_back({tr->language(), tr->body()});
        }
        return result;
    }

    std::string env(const char* name, const char* fallback = "") {
        const char* value = std::getenv(name);
        return value ? value : fallback;
    }
}

TEST(MarkdownParser, EmptyAndUnparsedDocumentsGiveNoBlocks) {
    EXPECT_TRUE(MarkdownParser().getElementsOfProject().empty());
    EXPECT_TRUE(parse("").empty());
}

TEST(MarkdownParser, SingleBlockDocumentTerminates) {
    auto blocks = parse("only text\n");
    ASSERT_EQ(blocks.size(), 1u);
    EXPECT_EQ(blocks[0].Language, "Markdown");
    auto code = parse("```sysml\npart a;\n```\n");
    ASSERT_EQ(code.size(), 1u);
    EXPECT_EQ(code[0].Language, "sysml");
    EXPECT_EQ(code[0].Body, "part a;\n");
}

TEST(MarkdownParser, BlocksComeInDocumentOrderAndHtmlIsNoCode) {
    auto blocks = parse("# Title\n\n```sysml\npart a;\n```\n\n<div>raw</div>\n\ntext\n\n```\nplain fence\n```\n");
    ASSERT_EQ(blocks.size(), 5u);
    EXPECT_EQ(blocks[0].Language, "Markdown");
    EXPECT_EQ(blocks[1].Language, "sysml");
    EXPECT_EQ(blocks[2].Language, "HTML");
    EXPECT_EQ(blocks[3].Language, "Markdown");
    EXPECT_EQ(blocks[4].Language, "");
    EXPECT_EQ(blocks[4].Body, "plain fence\n");
}

namespace {
    class MarkdownLive : public ::testing::Test {
    protected:
        void SetUp() override {
            const std::string url = env("DT_TEST_SERVER_URL");
            if (url.empty())
                GTEST_SKIP() << "DT_TEST_SERVER_URL is not set, the live test is skipped.";
            Service = std::make_unique<BACKEND_COMMUNICATION::CommunicationService>(url, "");
            ASSERT_TRUE(Service->setUserForLoginInBackend(env("DT_TEST_USER", "admin"), env("DT_TEST_PASSWORD")));
        }
        std::unique_ptr<BACKEND_COMMUNICATION::CommunicationService> Service;
    };
}

TEST_F(MarkdownLive, RealMarkdownFilesKeepTheirCodeInDocumentOrder) {
    const std::string suffix = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    int files = 0;
    for (const char* name : {"Eingine_Testing_Environment.md", "MQTT_Broker_Sicherheitskonzept.md"}) {
        const std::string path = std::string(DT_MARKDOWN_DIR) + "/" + name;
        std::ifstream file(path, std::ios::ate | std::ios::binary);
        if (!file || file.tellg() <= 0)
            continue; // missing (the MQTT document is not tracked by git) or empty (the engine document is empty)
        ++files;
        MarkdownParser parser;
        parser.parseMarkdownFile(QString::fromStdString(path));
        const auto elements = parser.getElementsOfProject();
        ASSERT_FALSE(elements.empty()) << name;

        // expected: the code of the document in document order, computed from the parser result alone
        std::string expected;
        std::size_t codeBlocks = 0;
        for (const auto& element : elements) {
            auto tr = std::dynamic_pointer_cast<TextualRepresentation>(element);
            if (!tr->body().empty() && tr->language() != "Markdown" && tr->language() != "YaML" && tr->language() != "HTML") {
                expected += tr->body();
                ++codeBlocks;
            }
        }

        auto project = Service->postProject(std::string("dt-md-") + name + "-" + suffix, "live test", "main");
        ASSERT_NE(project, nullptr);
        auto documentCommit = DigitalTwin::Model::createDocumentCommit(project->getName(), elements);
        auto commit = Service->postCommitWithId(project->getId(), documentCommit.Request);
        ASSERT_NE(commit, nullptr);

        auto downloaded = Service->getAllElementsOfCommit(project->getId(), commit->getId());
        EXPECT_EQ(downloaded.size(), 1u + 2u * elements.size()) << name;
        std::vector<std::string> warnings;
        EXPECT_EQ(DigitalTwin::Model::assembleModelCode(downloaded, &warnings), expected) << name;
        EXPECT_TRUE(warnings.empty()) << name;
        EXPECT_GT(codeBlocks, 0u) << name;
    }
    EXPECT_GT(files, 0);
}
