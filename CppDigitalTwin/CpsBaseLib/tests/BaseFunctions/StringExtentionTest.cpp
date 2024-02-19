//
// Created by Moritz Herzog on 19.02.24.
//
#include "../../src/BaseFuctions/StringExtention.hpp"
#include <gtest/gtest.h>

TEST(StringExtention,SplittingString_Valid) {
    std::string contentString = "This is a test.";
    char delimiter = ' ';

    std::vector<std::string> resultValue = CPSBASELIB::STD_EXTENTION::StringExtention::splitString(contentString,delimiter);
    EXPECT_EQ(4,resultValue.size());
    EXPECT_EQ("This", resultValue[0]);
    EXPECT_EQ("is", resultValue[1]);
    EXPECT_EQ("a", resultValue[2]);
    EXPECT_EQ("test.", resultValue[3]);
}

TEST(StringExtention,SplittingString_SignNotInContent) {
    std::string contentString = "This is a test.";
    char delimiter = ';';

    std::vector<std::string> resultValue = CPSBASELIB::STD_EXTENTION::StringExtention::splitString(contentString,delimiter);
    EXPECT_EQ(1,resultValue.size());
}