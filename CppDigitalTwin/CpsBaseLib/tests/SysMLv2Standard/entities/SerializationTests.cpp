//
// Created by Moritz Herzog on 19.02.24.
//
#include <gtest/gtest.h>
#include <boost/uuid/uuid_generators.hpp>
#include <SysMLv2Standard/entities/Identity.h>

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