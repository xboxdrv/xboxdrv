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

#ifndef HEADER_STRUT_SPLIT_HPP
#define HEADER_STRUT_SPLIT_HPP

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

namespace strut {

void split_at(std::string const& str, char delimiter, std::string* lhs, std::string* rhs);

inline
std::vector<std::string> split(std::string_view text, std::string_view delimiter)
{
  auto is_delimiter = [delimiter](char c) -> bool {
    return std::find(delimiter.begin(), delimiter.end(), c) != delimiter.end();
  };

  std::vector<std::string> result;

  std::string::size_type start = 0;
  for(std::string::size_type i = 0; i != text.size(); ++i)
  {
    if (is_delimiter(text[i]))
    {
      result.emplace_back(text.substr(start, i - start));
      start = i + 1;
    }
  }

  result.emplace_back(text.substr(start));

  return result;
}

inline
std::vector<std::string> split(std::string_view text, char delimiter)
{
  std::vector<std::string> result;

  std::string::size_type start = 0;
  for(std::string::size_type i = 0; i != text.size(); ++i)
  {
    if (text[i] == delimiter)
    {
      result.emplace_back(text.substr(start, i - start));
      start = i + 1;
    }
  }

  result.emplace_back(text.substr(start));

  return result;
}

class splitter
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
      m_parent(nullptr), m_start(0), m_end(0)
    {}

    iterator(splitter const* parent) :
      m_parent(parent), m_start(0), m_end(0)
    {
      auto const& text = m_parent->m_text;
      auto const& delimiter = m_parent->m_delimiter;

      m_end = text.find(delimiter);
      if (m_end == std::string_view::npos) {
        m_end = text.size();
      }
    }

    iterator(iterator const& other) = default;
    iterator& operator=(iterator const& other) = default;

    bool operator==(iterator const& other) const = default;
    bool operator!=(iterator const& other) const = default;

    iterator& operator++() {
      auto const& text = m_parent->m_text;
      auto const& delimiter = m_parent->m_delimiter;

      if (m_end == text.size()) {
        *this = iterator();
        return *this;
      } else {
        m_start = m_end + 1;
        m_end = text.find(delimiter, m_start);
        if (m_end == std::string_view::npos) {
          m_end = text.size();
        }
        return *this;
      }
    }

    std::string_view operator*() const {
      return m_parent->m_text.substr(m_start, m_end - m_start);
    }

  private:
    splitter const* m_parent;
    std::string_view::size_type m_start;
    std::string_view::size_type m_end;
  };

public:
  splitter(std::string_view text, char delimiter) :
    m_text(text),
    m_delimiter(delimiter)
  {}

  splitter(splitter const& other) = default;
  splitter& operator=(splitter const& other) = default;

  iterator begin() const { return iterator(this); }
  iterator end() const { return iterator(); }

private:
  std::string_view m_text;
  char m_delimiter;
};


} // namespace strut

#endif

/* EOF */
