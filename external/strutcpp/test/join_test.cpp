#include <gtest/gtest.h>

#include <strut/join.hpp>

TEST(JoinTest, join)
{
  EXPECT_EQ(strut::join(std::vector<std::string>{"1", "2", "3"}, ","), "1,2,3");
  EXPECT_EQ(strut::join(std::vector<std::string>{"1", "2", "3"}, "-+-"), "1-+-2-+-3");
  EXPECT_EQ(strut::join(std::vector<std::string>{"1", "2", "3"}, ""), "123");
  EXPECT_EQ(strut::join(std::vector<std::string>{}, ","), "");
}

/* EOF */

