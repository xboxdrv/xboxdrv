#include <gtest/gtest.h>
#include <iostream>
#include <sstream>

#include "ini/ini_parser.hpp"
#include "ini/ini_builder.hpp"

class INIParserTestBuilder : public INIBuilder
{
private:
  std::ostream& m_out;

public:
  INIParserTestBuilder(std::ostream& out) :
    m_out(out)
  {}

  void send_section(const std::string& section)
  {
    m_out << "[" << section << "]" << std::endl;
  }

  void send_pair(const std::string& name, const std::string& value)
  {
    m_out << "\"" << name << "\" = \"" << value << "\"" << std::endl;
  }
};

TEST(INIParserTest, parse)
{
  std::string input =
    "\r\n[test]\n"
    "name = value1 # comment\r\n"
    "name = value2\n"
    "  name   =    value3   \n"
    "name#foo = value#foo # comment\n"
    "\n"
    "   name#foo     = value#foo # comment\r\n"
    "   name#foo     =        value#foo          # comment\n"
    "  name#empty \n"
    "  name#empty =\n"
    "\"name#not-empty\" = \"value#foo\" # comment\n"
    "  foo    bar     =       oing    # foo";

  std::string expected =
    "[test]\n"
    "\"name\" = \"value1\"\n"
    "\"name\" = \"value2\"\n"
    "\"name\" = \"value3\"\n"
    "\"name#foo\" = \"value#foo\"\n"
    "\"name#foo\" = \"value#foo\"\n"
    "\"name#foo\" = \"value#foo\"\n"
    "\"name#empty\" = \"\"\n"
    "\"name#empty\" = \"\"\n"
    "\"name#not-empty\" = \"value#foo\"\n"
    "\"foo    bar\" = \"oing\"\n";

  std::istringstream in(input);
  std::ostringstream out;
  INIParserTestBuilder builder(out);
  INIParser parser(in, builder, "<from-string>");
  parser.run();

  ASSERT_EQ(expected, out.str());
}

/* EOF */
