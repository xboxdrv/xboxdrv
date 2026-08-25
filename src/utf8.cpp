// Pingus - A free Lemmings clone
// Copyright (C) 2008 Matthias Braun <matze@braunis.de>,
//                    Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// decode_utf8() came from supertux - 403d6c74ccdac66ffa49a363edaf22c8374c56c5

#include "utf8.hpp"

#include <iostream>
#include <stdexcept>

namespace strut {
namespace utf8 {

namespace {
/** Replacement character for invalid UTF-8 sequences */
constexpr uint32_t INVALID_UTF8_SEQUENCE = 0xFFFD;
} // namespace

bool
is_linebreak_character(uint32_t unicode)
{
  return unicode == ' ' || unicode >= 0x3400;
}

std::string::size_type
length(std::string_view str)
{
  // Not checking for valid UTF-8 sequences should be ok, since
  // incorrect ones are a character too.
  std::string::size_type len = 0;
  for(std::string_view::const_iterator i = str.begin(); i != str.end(); ++i)
  {
    unsigned char c = static_cast<unsigned char>(*i);
    if (((c & 0xc0) == 0xc0) || (c < 0x80)) // 0xc0 == 1100_000
    {
      len += 1;
    }
  }

  return len;
}

std::string
substr(iterator const& first, iterator const& last)
{
  return std::string(first.get_string().substr(first.get_index(),
                                               last.get_index() - first.get_index()));
}

std::string
substr(std::string_view text, std::string::size_type pos, std::string::size_type n)
{
  std::string_view::const_iterator beg_it = advance(text.begin(), pos);
  std::string_view::const_iterator end_it = advance(beg_it, n);

  return std::string(beg_it, end_it);
}

std::string_view::const_iterator
advance(std::string_view::const_iterator it, std::string::size_type n)
{
  for(std::string::size_type i = 0; i < n; ++i)
  {
    // FIXME: Doesn't check if UTF8 sequence is valid
    unsigned char c = static_cast<unsigned char>(*it);

    if (c < 0x80)
    {
      it += 1;
    }
    else if ((c & 0xf0) == 0xf0)
    {
      it += 4;
    }
    else if ((c & 0xe0) == 0xe0)
    {
      it += 3;
    }
    else if ((c & 0xc0) == 0xc0)
    {
      it += 2;
    }
    else
    {
      std::cerr << "UTF8: malformed UTF-8 sequence: " << static_cast<int>(c) << std::endl;
      it += 1;
    }
  }

  return it;
}

/** returns true if this byte matches a bitmask of 10xx.xxxx, i.e. it
    is the 2nd, 3rd or 4th byte of a multibyte utf8 string */
bool
has_multibyte_mark(unsigned char c)
{
  return ((c & 0300) == 0200);
}

uint32_t
decode_utf8(std::string_view text)
{
  size_t p = 0;
  return decode_utf8(text, p);
}

/**
 * gets unicode character at byte position @a p of UTF-8 encoded @a
 * text, then advances @a p to the next character.
 *
 * @throws std::runtime_error if decoding fails.
 * See unicode standard section 3.10 table 3-5 and 3-6 for details.
 */
uint32_t
decode_utf8(std::string_view text, size_t& p)
{
  unsigned char c1 = static_cast<unsigned char>(text[p+0]);

  if (has_multibyte_mark(c1))
  {
    throw std::runtime_error("Malformed utf-8 sequence");
  }
  else if ((c1 & 0200) == 0000)
  {
    // 0xxx.xxxx: 1 byte sequence
    p+=1;

    return c1;
  }
  else if ((c1 & 0340) == 0300)
  {
    // 110x.xxxx: 2 byte sequence
    if(p+1 >= text.size()) throw std::range_error("Malformed utf-8 sequence");
    unsigned char c2 = static_cast<unsigned char>(text[p+1]);
    if (!has_multibyte_mark(c2)) throw std::runtime_error("Malformed utf-8 sequence");
    p+=2;

    return (c1 & 0037u) << 6 | (c2 & 0077u);
  }
  else if ((c1 & 0360u) == 0340u)
  {
    // 1110.xxxx: 3 byte sequence
    if(p+2 >= text.size()) throw std::range_error("Malformed utf-8 sequence");
    unsigned char c2 = static_cast<unsigned char>(text[p+1]);
    unsigned char c3 = static_cast<unsigned char>(text[p+2]);
    if (!has_multibyte_mark(c2)) throw std::runtime_error("Malformed utf-8 sequence");
    if (!has_multibyte_mark(c3)) throw std::runtime_error("Malformed utf-8 sequence");
    p+=3;

    return (c1 & 0017u) << 12 | (c2 & 0077u) << 6 | (c3 & 0077u);
  }
  else if ((c1 & 0370u) == 0360u)
  {
    // 1111.0xxx: 4 byte sequence
    if(p+3 >= text.size()) throw std::range_error("Malformed utf-8 sequence");
    unsigned char c2 = static_cast<unsigned char>(text[p+1]);
    unsigned char c3 = static_cast<unsigned char>(text[p+2]);
    unsigned char c4 = static_cast<unsigned char>(text[p+4]);
    if (!has_multibyte_mark(c2)) throw std::runtime_error("Malformed utf-8 sequence");
    if (!has_multibyte_mark(c3)) throw std::runtime_error("Malformed utf-8 sequence");
    if (!has_multibyte_mark(c4)) throw std::runtime_error("Malformed utf-8 sequence");
    p+=4;

    return (c1 & 0007u) << 18 | (c2 & 0077u) << 12 | (c3 & 0077u) << 6 | (c4 & 0077u);
  }
  else
  {
    throw std::runtime_error("Malformed utf-8 sequence");
  }
}

std::string
encode_utf8(uint32_t unicode)
{
  std::string result;
  if (unicode < 0x80)
  {
    result += static_cast<char>(unicode);
  }
  else if (unicode < 0x800)
  {
    result += static_cast<char>((unicode >> 6) | 0xc0);
    result += static_cast<char>((unicode & 0x3f) | 0x80);
  }
  else if (unicode < 0x10000)
  {
    result += static_cast<char>((unicode  >> 12) | 0xe0);
    result += static_cast<char>(((unicode >> 6) & 0x3f) | 0x80);
    result += static_cast<char>((unicode & 0x3f) | 0x80);
  }
  else
  {
    result += static_cast<char>((unicode  >> 18) | 0xf0);
    result += static_cast<char>(((unicode >> 12) & 0x3f) | 0x80);
    result += static_cast<char>(((unicode >>  6) & 0x3f) | 0x80);
    result += static_cast<char>((unicode & 0x3f) | 0x80);
  }
  return result;
}

// FIXME: Get rid of exceptions in this code
iterator::iterator(std::string_view text_) :
  text(text_),
  pos(0),
  idx(0),
  chr(INVALID_UTF8_SEQUENCE)
{
}

iterator
iterator::operator+(int n)
{
  iterator it = *this;
  for(int i = 0; i < n && it.next(); ++i);
  return it;
}

bool
iterator::next()
{
  try
  {
    idx = pos;
    chr = decode_utf8(text, pos);
  }
  catch (std::exception&)
  {
    std::cerr << "Malformed utf-8 sequence beginning with {} found "
              << *(reinterpret_cast<uint32_t const*>(text.data() + pos))
              << std::endl;
    chr = INVALID_UTF8_SEQUENCE;
    ++pos;
  }

  return pos <= text.size();
}

uint32_t
iterator::operator*() const
{
  return chr;
}

} // namespace utf8
} // namespace strut

/* EOF */
