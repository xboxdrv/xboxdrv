#include <gtest/gtest.h>

#include <strut/split.hpp>

TEST(SplitAt, split_at)
{
  {
    std::string lhs, rhs;
    strut::split_at("Hello World", ' ', &lhs, &rhs);
    EXPECT_EQ(lhs, "Hello");
    EXPECT_EQ(rhs, "World");
  }

  {
    std::string lhs, rhs;
    strut::split_at("", ' ', &lhs, &rhs);
    EXPECT_EQ(lhs, "");
    EXPECT_EQ(rhs, "");
  }
}

using V = std::vector<std::string>;
using T = std::tuple<std::string, char, V>;

class SplitTest : public ::testing::TestWithParam<T>
{
public:
  SplitTest() = default;
};

TEST_P(SplitTest, split)
{
  auto const& [text, delimiter, expected] = GetParam();
  EXPECT_EQ(strut::split(text, delimiter), expected);
}

TEST_P(SplitTest, splitter)
{
  auto const& [text, delimiter, expected] = GetParam();
  strut::splitter splitter(text, delimiter);
  EXPECT_EQ(V(splitter.begin(), splitter.end()), expected);
}

INSTANTIATE_TEST_CASE_P(
  ParamSplitTest, SplitTest,
  ::testing::Values(
    T{"a:b:c:d", ':', {"a", "b", "c", "d"}},
    T{"a:b:c:d", ':', {"a", "b", "c", "d"}},
    T{":a:b:c:d:", ':', {"", "a", "b", "c", "d", ""}},
    T{"", ':', {""}},
    T{":", ':', {"", ""}},
    T{"::", ':', {"", "", ""}},
    T{"abc:", ':', {"abc", ""}},
    T{":xyz", ':', {"", "xyz"}},
    T{"abc", ':', {"abc"}}));

/* EOF */
