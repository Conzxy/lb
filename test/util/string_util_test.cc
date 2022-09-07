#include "lb/util/string_util.h"

#include <gtest/gtest.h>

using namespace util;

TEST(string_util, strcat) {
  auto str = StrCat("A", "B", "C", "D");
  EXPECT_EQ(str, "ABCD");
}
