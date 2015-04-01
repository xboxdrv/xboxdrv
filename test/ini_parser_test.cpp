#include <iostream>
#include <sstream>

#include "ini_parser.hpp"
#include "ini_builder.hpp"

class INIParserTestBuilder : public INIBuilder
{
public:
  void send_section(const std::string& section)
  {
    std::cout << "[" << section << "]" << std::endl;
  }

  void send_pair(const std::string& name, const std::string& value)
  {
    std::cout << "\"" << name << "\" = \"" << value << "\"" << std::endl;
  }
};

int main()
try
{
  std::istringstream in(
    "[test]\n"
    "name = value1 # comment\n"
    "name = value2\n"
    "  name   =    value3   \n"
    "name#foo = value#foo # comment\n"
    "\n"
    "   name#foo     = value#foo # comment\n"
    "   name#foo     =        value#foo          # comment\n"
    "  name#empty \n"
    "  name#empty =\n"
    "\"name#not-empty\" = \"value#foo\" # comment\n"
    "  foo    bar     =       oing    # foo"
    );

  INIParserTestBuilder builder;
  INIParser parser(in, builder, "test");
  parser.run();

  return 0;
}
catch(const std::exception& err)
{
  std::cout << "exception: " << err.what() << std::endl;
}

/* EOF */
