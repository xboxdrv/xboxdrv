// strut - Collection of string utilities
// Copyright (C) 2020 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
// License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#ifndef HEADER_STRUT_TOKENIZE_HPP
#define HEADER_STRUT_TOKENIZE_HPP

#include <algorithm>
#include <string_view>
#include <vector>

namespace strut {

inline
std::vector<std::string> tokenize(std::string_view text, std::string_view delimiter)
{
  auto is_delimiter = [delimiter](char c) -> bool {
    return std::find(delimiter.begin(), delimiter.end(), c) != delimiter.end();
  };

  std::vector<std::string> result;

  for(std::string::size_type i = 0; i != text.size();)
  {
    while(is_delimiter(text[i]) && i != text.size()) { ++i; };
    const std::string::size_type start = i;
    while(!is_delimiter(text[i]) && i != text.size()) { ++i; };
    const std::string::size_type end = i;
    if (start != end) {
      result.emplace_back(text.substr(start, end - start));
    }
  }

  return result;
}

inline
std::vector<std::string> tokenize(std::string_view text, char delimiter)
{
  std::vector<std::string> result;

  for(std::string::size_type i = 0; i != text.size();)
  {
    while(text[i] == delimiter && i != text.size()) { ++i; }
    const std::string::size_type start = i;
    while(text[i] != delimiter && i != text.size()) { ++i; }
    const std::string::size_type end = i;
    if (start != end) {
      result.emplace_back(text.substr(start, end - start));
    }
  }

  return result;
}

class tokenizer
{
public:
  friend class iterator;

  class iterator {
  public:
    using iterator_category = std::input_iterator_tag;
    using value_type = std::string_view;
    using difference_type = void;
    using pointer = void;
    using reference = void;

  public:
    iterator() :
      m_parent(nullptr), m_i(0), m_start(0), m_end(0)
    {}

    iterator(tokenizer const* parent) :
      m_parent(parent), m_i(0), m_start(0), m_end(0)
    {
      operator++();
    }

    iterator(iterator const& other) = default;
    iterator& operator=(iterator const& other) = default;

    bool operator==(iterator const& other) const = default;
    bool operator!=(iterator const& other) const = default;

    iterator& operator++() {
      if (m_i == m_parent->m_text.size()) {
        return *this = iterator();
      } else {
        while(m_parent->m_text[m_i] == m_parent->m_delimiter && m_i != m_parent->m_text.size()) { ++m_i; }
        m_start = m_i;
        while(m_parent->m_text[m_i] != m_parent->m_delimiter && m_i != m_parent->m_text.size()) { ++m_i; }
        m_end = m_i;

        if (m_start == m_end) {
          operator++();
        }

        return *this;
      }
    }

    std::string_view operator*() const {
      return m_parent->m_text.substr(m_start, m_end - m_start);
    }

  private:
    tokenizer const* m_parent;
    std::string_view::size_type m_i;
    std::string_view::size_type m_start;
    std::string_view::size_type m_end;
  };

public:
  tokenizer(std::string_view text, char delimiter) :
    m_text(text),
    m_delimiter(delimiter)
  {}

  tokenizer(tokenizer const& other) = default;
  tokenizer& operator=(tokenizer const& other) = default;

  iterator begin() const { return iterator(this); }
  iterator end() const { return iterator(); }

private:
  std::string_view m_text;
  char m_delimiter;
};

} // namespace strut

#endif

/* EOF */
