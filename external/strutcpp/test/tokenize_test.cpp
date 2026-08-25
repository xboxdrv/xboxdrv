#include <iostream>
#include <gtest/gtest.h>

#include "strut/tokenize.hpp"

using V = std::vector<std::string>;
using T = std::tuple<std::string, char, V>;

class TokenizeTest : public ::testing::TestWithParam<T>
{
public:
  TokenizeTest() = default;
};

TEST_P(TokenizeTest, tokenize)
{
  auto const& [text, delimiter, expected] = GetParam();
  EXPECT_EQ(strut::tokenize(text, delimiter), expected);
}

TEST_P(TokenizeTest, tokenizer)
{
  auto const& [text, delimiter, expected] = GetParam();
  strut::tokenizer tokenizer(text, delimiter);
  EXPECT_EQ(V(tokenizer.begin(), tokenizer.end()), expected);
}

INSTANTIATE_TEST_CASE_P(
  ParamTokenizeTest, TokenizeTest,
  ::testing::Values(
    T{"a b c d", ' ', {"a", "b", "c", "d"}},
    T{"  a1   b2  c3  d4  ", ' ', {"a1", "b2", "c3", "d4"}},
    T{"xxa1xxxb2xxc3xxd4xxx", 'x', {"a1", "b2", "c3", "d4"}},
    T{"abc", ' ', {"abc"}},
    T{"   abc", ' ', {"abc"}},
    T{" abc", ' ', {"abc"}},
    T{"abc ", ' ', {"abc"}},
    T{"abc    ", ' ', {"abc"}},
    T{"   ", ' ', {}},
    T{"", ' ', {}}
    ));

/* EOF */
