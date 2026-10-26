#include <gtest/gtest.h>

#include <stdexcept>
#include <sysmlv2/rest/entities/Commit.h>
#include <sysmlv2/rest/entities/Project.h>
#include <sysmlv2/service/online/ConnectionError.h>
#include <sysmlv2/service/online/HttpException.h>

#include "BECommunicationService.h"
#include "entities/TwinRequest.h"

using BACKEND_COMMUNICATION::CommunicationService;
using SysMLv2::API::EXCEPTIONS::HTTPException;

TEST(TwinRequest, WithoutCommitSerializationThrows) {
    SysMLv2::REST::TwinRequest request("twin", nullptr);
    EXPECT_THROW(request.serializeToJson(), std::invalid_argument);
}

TEST(TwinRequest, WithCommitSerializes) {
    auto project = std::make_shared<SysMLv2::REST::Project>("p", "d", "main");
    auto commit = std::make_shared<SysMLv2::REST::Commit>("c", project);
    SysMLv2::REST::TwinRequest request("twin", commit);
    auto json = request.serializeToJson();
    EXPECT_NE(json.find("\"twin\""), std::string::npos);
}

TEST(DescribeBackendError, BadRequestContainsTheServerMessage) {
    HTTPException ex(400, R"({"error":"change must not be empty"})");
    EXPECT_EQ(CommunicationService::describeBackendError(ex), "The request was rejected as invalid: change must not be empty");
}

TEST(DescribeBackendError, BadRequestWithoutBody) {
    HTTPException ex(400, "");
    EXPECT_EQ(CommunicationService::describeBackendError(ex), "The request was rejected as invalid.");
    HTTPException notJson(400, "<html>");
    EXPECT_EQ(CommunicationService::describeBackendError(notJson), "The request was rejected as invalid.");
}

TEST(DescribeBackendError, ServerMessageIsAppendedToTheOtherCodes) {
    EXPECT_EQ(CommunicationService::describeBackendError(HTTPException(404, R"({"error":"project not found"})")),
              "The requested resource was not found or you have no access to it. (project not found)");
    EXPECT_EQ(CommunicationService::describeBackendError(HTTPException(409, R"({"error":"branch exists"})")),
              "The request conflicts with the current state on the server. (branch exists)");
    EXPECT_EQ(CommunicationService::describeBackendError(HTTPException(404, "")),
              "The requested resource was not found or you have no access to it.");
    EXPECT_EQ(CommunicationService::describeBackendError(HTTPException(500, R"({"error":"boom"})")), "HTTP 500: boom");
}

TEST(DescribeBackendError, PasswordChangeAndOtherExceptionsAreUnchanged) {
    EXPECT_EQ(CommunicationService::describeBackendError(HTTPException(403, R"({"error":"password_change_required"})")),
              "You must change your password before you can continue. Please do so in the web interface.");
    EXPECT_EQ(CommunicationService::describeBackendError(std::runtime_error("plain")), "plain");
}
