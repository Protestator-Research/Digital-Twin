//
// Created by Moritz Herzog on 19.02.24.
//

#include <gtest/gtest.h>
#include <boost/uuid/uuid_generators.hpp>
#include <SysMLv2Standard/entities/DataIdentity.h>
#include <SysMLv2Standard/entities/Project.h>
#include <SysMLv2Standard/entities/Branch.h>
#include <SysMLv2Standard/SysMLv2Deserializer.h>

/**
 * Tests the serilization of the object containing a valid uuid.
 */
TEST(TestSerilization,TestSerilizationItentityNonEmptyId) {
    std::string jsonString = "{\n"
                             "  \"@id\": \"781f769c-b7f8-4b38-a732-592fda775626\"\n"
                             "}";
    SysMLv2::Entities::DataIdentity identity = SysMLv2::Entities::DataIdentity(boost::uuids::string_generator()("781f769c-b7f8-4b38-a732-592fda775626"));
    EXPECT_EQ(jsonString,identity.serializeToJson());
}

/**
 * Tests Se
 */
TEST(TestDeserialization, TestDeserializationIdentity){
    std::string jsonString = "{\n"
                             "  \"@id\": \"781f769c-b7f8-4b38-a732-592fda775626\"\n"
                             "}";
    SysMLv2::Entities::DataIdentity identity = SysMLv2::Entities::DataIdentity("{\"@id\":\"781f769c-b7f8-4b38-a732-592fda775626\"}");
    EXPECT_EQ(jsonString,identity.serializeToJson());
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

    //EXPECT_TRUE(SysMLv2::Entities::Branch("{ \"@id\": \"3fa85f64-5717-4562-b3fc-2c963f66afa6\" }") == *(project.getDefaultBranch()));
}

TEST(TestDeserialization, TestDeserilizationCrashIssueWithout) {
    try {
        std::string jsonString = "[{\"id\":\"e1a5a414-b6c8-483d-8e90-d6ec46dbc590\",\"name\":\"ScalarValues\",\"description\":\"Pre-loaded project ScalarValues\",\"defaultId\":\"e2706ccd-6f92-4693-9c0a-207ac23f3d8d\",\"commitIDs\":[\"e2706ccd-6f92-4693-9c0a-207ac23f3d8d\"],\"headIDs\":[\"e2706ccd-6f92-4693-9c0a-207ac23f3d8d\"]},{\"id\":\"7fae8873-377c-4cc6-bf16-e8b8eaf2901f\",\"name\":\"ISO26262\",\"description\":\"Pre-loaded project ISO26262\",\"defaultId\":\"883c3982-3236-4822-adcb-64dacc3ac618\",\"commitIDs\":[\"883c3982-3236-4822-adcb-64dacc3ac618\"],\"headIDs\":[\"883c3982-3236-4822-adcb-64dacc3ac618\"]},{\"id\":\"93d9c988-287b-4650-9c98-a6479d79df62\",\"name\":\"SI\",\"description\":\"Pre-loaded project SI\",\"defaultId\":\"becdb5cb-a648-431a-b418-efa9d92e1a1d\",\"commitIDs\":[\"becdb5cb-a648-431a-b418-efa9d92e1a1d\"],\"headIDs\":[\"becdb5cb-a648-431a-b418-efa9d92e1a1d\"]},{\"id\":\"6baa5514-9a70-4f1f-99a3-8df8f3a7c1c8\",\"name\":\"Math\",\"description\":\"Pre-loaded project Math\",\"defaultId\":\"b2106e40-f0cb-42a0-9080-2493655e7a78\",\"commitIDs\":[\"b2106e40-f0cb-42a0-9080-2493655e7a78\"],\"headIDs\":[\"b2106e40-f0cb-42a0-9080-2493655e7a78\"]},{\"id\":\"f1a1325f-fb0b-47e7-883f-0416872ea3e4\",\"name\":\"Context\",\"description\":\"Pre-loaded project Context\",\"defaultId\":\"0b6702e7-bf3a-4f10-879b-cd445a03f5ed\",\"commitIDs\":[\"0b6702e7-bf3a-4f10-879b-cd445a03f5ed\"],\"headIDs\":[\"0b6702e7-bf3a-4f10-879b-cd445a03f5ed\"]},{\"id\":\"ff58511c-e8dc-42b2-8a97-29ebf38e1d32\",\"name\":\"GenialBusses\",\"description\":\"Pre-loaded project GenialBusses\",\"defaultId\":\"2edbecf1-ed43-4d3f-b616-973c11bf7861\",\"commitIDs\":[\"2edbecf1-ed43-4d3f-b616-973c11bf7861\"],\"headIDs\":[\"2edbecf1-ed43-4d3f-b616-973c11bf7861\"]}]";
        SysMLv2::SysMLv2Deserializer::deserializeJsonArray(jsonString);
        FAIL()<<"Expected Exception!";
    }catch (std::exception& ex) {
        std::cout<<ex.what()<<std::endl;
        EXPECT_EQ(ex.what(),std::string("[json.exception.type_error.302] type must be string, but is null"));
    }
}

TEST(TestSerilization, TestSerilizationAndDeserialization) {
    std::string jsonString = "{\n"
                             "  \"@id\": \"3fa85f64-5717-4562-b3fc-2c963f66afa6\",\n"
                             "  \"@type\": \"Project\",\n"
                             "  \"created\": \"2024-02-22T08:58:57.343000000Z\",\n"
                             "  \"defaultBranch\": {\n"
                             "    \"@id\": \"3fa85f64-5717-4562-b3fc-2c963f66afa6\"\n"
                             "  },\n"
                             "  \"description\": \"string\",\n"
                             "  \"name\": \"string\"\n"
                             "}";

    SysMLv2::Entities::Project project = SysMLv2::Entities::Project(jsonString);

    //EXPECT_EQ(jsonString,project.serializeToJson());
}
