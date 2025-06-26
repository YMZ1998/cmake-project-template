#include <gtest/gtest.h>

int add(int a, int b) {
  return a + b;
}

TEST(AdditionTests, BasicAdd) {
  EXPECT_EQ(add(1, 2), 3);
  EXPECT_EQ(add(-1, 1), 0);
  EXPECT_EQ(add(0, 0), 0);
}
