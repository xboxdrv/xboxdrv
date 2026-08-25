#include <gtest/gtest.h>

#include "strut/utf8.hpp"

TEST(UTF8Test, length)
{
  EXPECT_EQ(strut::utf8::length("hello"), 5);
  EXPECT_EQ(strut::utf8::length(reinterpret_cast<char const*>(u8"hällö")), 5);
}

/* EOF */
