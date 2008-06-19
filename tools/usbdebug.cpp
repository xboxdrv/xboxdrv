#include <string>
#include <vector>
#include <iostream>

bool has_prefix(const std::string& lhs, const std::string rhs)
{
  if (lhs.length() < rhs.length())
    return false;
  else
    return lhs.compare(0, rhs.length(), rhs) == 0;
}

bool readline(const std::string& prompt,
              std::string& line)
{
  std::cout << prompt << std::flush;
  bool ret = !std::getline(std::cin, line).eof();
  //line = strip(line);
  return ret;
}

std::vector<std::string> 
tokenize(const std::string& str, const std::string& delimiters = " ")
{
  std::vector<std::string> tokens;  

  // Skip delimiters at beginning.
  std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
  // Find first "non-delimiter".
  std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

  while (std::string::npos != pos || std::string::npos != lastPos)
    {
      // Found a token, add it to the vector.
      tokens.push_back(str.substr(lastPos, pos - lastPos));
      // Skip delimiters.  Note the "not_of"
      lastPos = str.find_first_not_of(delimiters, pos);
      // Find next "non-delimiter"
      pos = str.find_first_of(delimiters, lastPos);
    }

  return tokens;
}

int main(int argc, char** argv)
{
  std::string line;
  bool quit = false;
  while (!quit && readline("\033[32musb\033[0m> ", line))
    {
      std::vector<std::string> args = tokenize(line);
      if (args.empty())
        {
        }
      else
        {
          if (0)
            {
              for(int i = 0; i < int(args.size()); ++i)
                {
                  std::cout << i << ":'" << args[i] << "' ";
                }
              std::cout << std::endl;
            }

          if (args[0] == "help")
            {
              std::cout << "help:   Print this help" << std::endl;
              std::cout << "quit:   Exit usbdebug" << std::endl;
              std::cout << "send:   Send data to an USB Endpoint" << std::endl;
            }
          else if (args[0] == "send")
            {
            }
          else if (args[0] == "quit")
            {
              std::cout << "Exiting" << std::endl;
              quit = true;
            }
          else
            {
              std::cout << "Unknown command '" << args[0] << "', type 'help' to list all available commands" << std::endl;
            }
        }
    }
  std::cout << std::endl;
  return 0;
}

/* EOF */
