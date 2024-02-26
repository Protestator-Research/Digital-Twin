//
// Created by Moritz Herzog on 23.02.24.
//
#include <nlohmann/json.hpp>
#include <gtest/gtest.h>
#include <SysMLv2Standard/entities/Record.h>

TEST(GenerationTests, TestGenerationRecordJSONString) {
    try {
        SysMLv2::Entities::Record record = SysMLv2::Entities::Record("  {\n"
                                                                     "    \"@id\": \"3fa85f64-5717-4562-b3fc-2c963f66afa6\",\n"
                                                                     "    \"@type\": \"string\",\n"
                                                                     "    \"name\": \"string\",\n"
                                                                     "    \"alias\": [\"string\"],\n"
                                                                     "    \"description\": \"string\"\n"
                                                                     "  }");
    }catch (...) {
        FAIL();
    }
    SUCCEED();
}

TEST(GenerationTests, TestGenerationRecordFalseJsonString) {
    try {
        SysMLv2::Entities::Record record = SysMLv2::Entities::Record("  {\n"
                                                                     "    \"@id\": \"3fa85f64-5717-4562-b3fc-2c963f66afa6\",\n"
                                                                     "    \"@type\": \"string\",\n"
                                                                     "    \"name\": \"string\",\n"
                                                                     "    \"alias\": [\"string\"],\n"
                                                                     "    \"description\": \"string\",\n"
                                                                     "  }");
    } catch (...) {
        FAIL();
    }
}

TEST(GenerationTests, TestGenerationRecordWithName) {
    try {
        SysMLv2::Entities::Record record = SysMLv2::Entities::Record("Test Project");
    }catch (...) {
        FAIL();
    }
    SUCCEED();
}