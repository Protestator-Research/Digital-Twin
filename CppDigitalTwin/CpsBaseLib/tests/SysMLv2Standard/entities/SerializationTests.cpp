//
// Created by Moritz Herzog on 19.02.24.
//

#include <gtest/gtest.h>
#include <boost/uuid/uuid_generators.hpp>
#include <SysMLv2Standard/entities/Identity.h>
#include <SysMLv2Standard/entities/Project.h>

/**
 * Tests the Serilization of the object that has no elements within it self.
 */
TEST(TestSerilization,TestSerilizationItentityEmptyId) {
    SysMLv2::Entities::Identity identity;
    EXPECT_EQ("null",identity.serializeToJson());
}

/**
 * Tests the serilization of the object containing a valid uuid.
 */
TEST(TestSerilization,TestSerilizationItentityNonEmptyId) {
    SysMLv2::Entities::Identity identity = SysMLv2::Entities::Identity(boost::uuids::string_generator()("781f769c-b7f8-4b38-a732-592fda775626"));
    EXPECT_EQ("{\"@id\":\"781f769c-b7f8-4b38-a732-592fda775626\"}",identity.serializeToJson());
}

/**
 * Tests Se
 */
TEST(TestDeserialization, TestDeserializationIdentity){
    SysMLv2::Entities::Identity identity = SysMLv2::Entities::Identity("{\"@id\":\"781f769c-b7f8-4b38-a732-592fda775626\"}");
    EXPECT_EQ("{\"@id\":\"781f769c-b7f8-4b38-a732-592fda775626\"}",identity.serializeToJson());
    EXPECT_EQ(boost::uuids::string_generator()("781f769c-b7f8-4b38-a732-592fda775626"),identity.getId());
}

TEST(TestSerilization, TestSerilizationProjectWithStringOfBackend) {
    SysMLv2::Entities::Project project = SysMLv2::Entities::Project("  {\n"
                                                                    "    \"@id\": \"3fa85f64-5717-4562-b3fc-2c963f66afa6\",\n"
                                                                    "    \"@type\": \"string\",\n"
                                                                    "    \"name\": \"string\",\n"
                                                                    "    \"description\": \"string\",\n"
                                                                    "    \"defaultBranch\": {\n"
                                                                    "      \"@id\": \"3fa85f64-5717-4562-b3fc-2c963f66afa6\"\n"
                                                                    "    },\n"
                                                                    "    \"created\": \"2024-02-22T08:58:57.343Z\"\n"
                                                                    "  }");

    EXPECT_EQ(boost::uuids::string_generator()("3fa85f64-5717-4562-b3fc-2c963f66afa6"),project.getId());
    EXPECT_EQ("string", project.getType());
    EXPECT_EQ("string", project.getName());
    EXPECT_EQ("string", project.getDescription());
    EXPECT_TRUE(SysMLv2::Entities::Identity("{ \"@id\": \"3fa85f64-5717-4562-b3fc-2c963f66afa6\" }") == project.getDefaultBranch());
}