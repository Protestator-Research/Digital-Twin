//
// Created by Moritz Herzog on 20.02.24.
//
#include <gtest/gtest.h>
#include "../../src/BaseFuctions/ListExtentions.h"

TEST(ListExtention,CheckIfListsAreEqualSuccess) {
    std::list<int> lhs = {1,2,3};
    std::list<int> rhs = {1,2,3};
    EXPECT_EQ(true, CPSBASELIB::STD_EXTENTION::LIST::areListsEqual<int>(lhs,rhs));
}