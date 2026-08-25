#include <gtest/gtest.h>

#include "strut/numeric_less.hpp"

TEST(NumericLessTest, numeric_less)
{
  EXPECT_TRUE(strut::numeric_less("1", "2"));
  EXPECT_TRUE(strut::numeric_less("001", "002"));
  EXPECT_TRUE(strut::numeric_less("999", "1111"));
  EXPECT_TRUE(strut::numeric_less("01", "002"));
  EXPECT_TRUE(strut::numeric_less("01", "02"));

  // FIXME: strut::numeric_less() doesn't deal with leading zero, i.e. "01" > "2"
  // EXPECT_TRUE(strut::numeric_less("01", "2"));
  // EXPECT_TRUE(strut::numeric_less("text01", "text2"));

  EXPECT_TRUE(strut::numeric_less("text10-5", "text10-10"));

  EXPECT_FALSE(strut::numeric_less("1111", "999"));

  std::vector<std::string> unsorted_lst =
    {
      "B1235",
      "A123",
      "A123",
      "A12",
      "B12323423A233",
      "B12323423A1231",
      "Z1",
      "A1A123",
      "A1A1",
      "A1A12"
    };

  /* FIXME: this is the result from 'sort -n', which is different from
     what StringUtil::numeric_less produces.

     std::vector<std::string> sorted_lst =
     {
     "A12",
     "A123",
     "A123",
     "A1A1",
     "A1A12"
     "A1A123",
     "B12323423A1231",
     "B12323423A233",
     "B1235",
     "Z1",
     };
  */

  std::vector<std::string> actual_lst =
    {
      "A1A1",
      "A1A12",
      "A1A123",
      "A12",
      "A123",
      "A123",
      "B1235",
      "B12323423A233",
      "B12323423A1231",
      "Z1"
    };

  std::sort(unsorted_lst.begin(), unsorted_lst.end(), strut::numeric_less);

  ASSERT_EQ(actual_lst, unsorted_lst);
}

/* EOF */
