/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_XBOXDRV_VIRTUALKEYBOARD_KEYBOARD_DESCRIPTION_HPP
#define HEADER_XBOXDRV_VIRTUALKEYBOARD_KEYBOARD_DESCRIPTION_HPP

#include <vector>
#include <string>
#include <assert.h>

class Key
{
public:
  enum Style { kLetter, kFunction, kModifier }; 
    
  //UIEventSequence m_sequence;
  int m_code;
  std::string m_label;
  std::string m_shift_label;
  std::string m_alt_label;
  Style m_style;
  int m_xspan;
  int m_yspan;

  Key() :
    m_code(-1),
    m_xspan(1),
    m_yspan(1)
  {}

  Key(int code,
      Style style,
      const std::string& label, 
      const std::string& shift_label = std::string(),
      const std::string& alt_label = std::string(),
      int xspan = 1,
      int yspan = 1) :
    m_code(code),
    m_style(style),
    m_label(label),
    m_shift_label(shift_label),
    m_alt_label(alt_label),
    m_xspan(xspan),
    m_yspan(yspan)
  {}

  operator bool() const
  {
    return (m_code != -1);
  }
};

class KeyboardDescription
{
public:
  static KeyboardDescription create_us_layout();

private:
  int m_width;
  int m_height;

  std::vector<Key> m_keys;

public:
  KeyboardDescription() {}
  KeyboardDescription(int width, int height);
  ~KeyboardDescription();

  int get_width() const  { return m_width; }
  int get_height() const { return m_height; }
  const Key& get_key(int x, int y) const { return m_keys[m_width * y + x]; }
  void set_key(int x, int y, const Key& key) 
  { 
    m_keys[m_width * y + x] = key; 
  }
};

#endif

/* EOF */
