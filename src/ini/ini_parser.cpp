/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2010 Ingo Ruhnke <grumbel@gmail.com>
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ini/ini_parser.hpp"

#include <stdexcept>

#include "ini/ini_builder.hpp"

INIParser::INIParser(std::istream& in, INIBuilder& builder, const std::string& context) :
  m_in(in),
  m_builder(builder),
  m_context(context),
  m_line(1),
  m_column(0),
  m_current_char(-1)
{}

void
INIParser::run()
{
  // read the first char
  next();

  while(peek() != -1)
  {
    if (accept('['))
    {
      m_builder.send_section(get_section());
      expect(']');
      whitespace();
      if (accept(';') || accept('#'))
        eat_rest_of_line();
      newline();
    }
    else if (accept(' ') || accept('\t') || accept('\n') || accept('\r'))
    {
      // eat whitespace
    }
    else if (accept(';') || accept('#'))
    {
      eat_rest_of_line();
      newline();
    }
    else // assume name=value pair
    {
      std::string name;
      std::string value;

      name = get_ident_or_string();
      whitespace();

      if (accept(';') || accept('#'))
      { // "name"
        eat_rest_of_line();
        newline();
      }
      else if (accept('='))
      {
        whitespace();
        if (accept(';') || accept('#'))
        { // "name = # comment"
          eat_rest_of_line();
          newline();
        }
        else
        { // "name = value"
          value = get_value_or_string();
          whitespace();

          if (accept(';') || accept('#'))
          { // "name = value # comment"
            eat_rest_of_line();
            newline();
          }
          else
          {
            newline();
          }
        }
      }

      m_builder.send_pair(name, value);
    }
  }
}

void
INIParser::error(const std::string& message)
{
  std::ostringstream str;
  str << m_context << ":" << m_line << ":" << m_column << ": error: " << message;
  throw std::runtime_error(str.str());
}

int
INIParser::peek()
{
  return m_current_char;
}

void
INIParser::next()
{
  if (m_in.eof())
  {
    error("unexpected end of file");
  }
  else
  {
    m_current_char = m_in.get();
    if (m_current_char == '\n')
    {
      m_line  += 1;
      m_column = 0;
    }
  }
}

bool
INIParser::accept(char c)
{
  if (peek() != c)
  {
    return false;
  }
  else
  {
    next();
    return true;
  }
}

void
INIParser::expect(char c)
{
  if (peek() != c)
  {
    std::ostringstream str;
    str << "expected '" << c << "', got ";
    if (peek() == -1)
      str << "EOF";
    else
      str << "'" << static_cast<char>(peek()) << "'";
    error(str.str());
  }
  else
  {
    next();
  }
}

std::string
INIParser::get_value_or_string()
{
  if (accept('"'))
  {
   std::string str = get_string();
   expect('"');
   return str;
  }
  else
  {
    return get_value();
  }
}

std::string
INIParser::get_ident_or_string()
{
  if (accept('"'))
  {
    std::string str = get_string();
    expect('"');
    return str;
  }
  else
  {
    return get_ident();
  }
}

std::string
INIParser::get_value()
{
  // an unquoted value is terminated either by a newline or a comment
  // character, whitespace at the end of the value will be trimmed
  std::ostringstream str;
  std::string::size_type last_char = std::string::npos;
  std::string::size_type cur = 0;
  char last_c = -1;
  while(peek() != '\n' &&
        peek() != -1 &&
        !((last_c == ' ' || last_c == '\t' || last_c == '\r') && (peek() == ';' || peek() == '#')))
  {
    if (peek() != ' ' && peek() != '\t' && peek() != '\r')
    {
      last_char = cur;
    }

    last_c = static_cast<char>(peek());
    str << last_c;

    next();
    cur += 1;
  }

  if (last_char == std::string::npos)
  {
    return str.str();
  }
  else
  {
    return str.str().substr(0, last_char + 1);
  }
}

std::string
INIParser::get_ident()
{
  // an unquoted value is terminated either by a newline or a comment
  // character, whitespace at the end of the value will be trimmed
  std::ostringstream str;
  std::string::size_type last_char = std::string::npos;
  std::string::size_type cur = 0;
  char last_c = -1;
  while(peek() != '\n' &&
        peek() != -1 &&
        peek() != '=' &&
        !((last_c == ' ' || last_c == '\t' || last_c == '\r') && (peek() == ';' || peek() == '#')))
  {
    if (peek() != ' ' && peek() != '\t' && peek() != '\r')
    {
      last_char = cur;
    }

    last_c = static_cast<char>(peek());
    str << last_c;

    next();
    cur += 1;
  }

  if (last_char == std::string::npos)
  {
    return str.str();
  }
  else
  {
    return str.str().substr(0, last_char + 1);
  }
}

std::string
INIParser::get_string()
{
  // reads a string, handles escaping, does not eat begin and end quotes
  std::ostringstream str;
  while(peek() != '"')
  {
    if (peek() == '\\')
    {
      next();
      switch(peek())
      {
        case '\\': str << '\\'; break;
        case '0': str << '\0'; break;
        case 'a': str << '\a'; break;
        case 'b': str << '\b'; break;
        case 't': str << '\t'; break;
        case 'r': str << '\r'; break;
        case 'n': str << '\n'; break;
        default: str << '\\' << static_cast<char>(peek()); break;
      }
    }
    else
    {
      str << static_cast<char>(peek());
    }
    next();
  }
  return str.str();
}

void
INIParser::newline()
{
  if (peek() == -1)
  {
    return;
  }
  else if (peek() != '\n')
  {
    error("expected newline");
  }
  else
  {
    next();
  }
}

void
INIParser::eat_rest_of_line()
{
  while(peek() != '\n' && peek() != -1)
  {
    next();
  }
}

std::string
INIParser::get_section()
{
  std::ostringstream str;
  while(peek() != ']')
  {
    str << static_cast<char>(peek());
    next();
  }
  return str.str();
}


void
INIParser::whitespace()
{
  while(peek() == ' ' || peek() == '\t' || peek() == '\r')
  {
    next();
  }
}

int
INIParser::getchar()
{
  return m_in.get();
}

/* EOF */
