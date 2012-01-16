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

#ifndef HEADER_XBOXDRV_BUTTON_COMBINATION_HPP
#define HEADER_XBOXDRV_BUTTON_COMBINATION_HPP

#include <bitset>
#include <string>
#include <vector>

#include "xboxmsg.hpp"

class ControllerMessageDescriptor;

class ButtonCombination
{
public:
  static ButtonCombination from_string(const std::string& str);

public:
  ButtonCombination();
  ButtonCombination(const std::string& button);
  ButtonCombination(const std::vector<std::string>& buttons);

  void init(const ControllerMessageDescriptor& desc);

  bool match(const std::bitset<256>& button_state) const;

  void print(std::ostream& os) const;

  /** Check if all buttons of \a this are also part of \a rhs. If
      ButtonCombination is empty, always returns false */
  bool is_subset_of(const ButtonCombination& rhs) const;

  bool operator==(const ButtonCombination&) const;

private:
  bool has_button(const std::string& button) const;

  int size() const;

  bool empty() const;

private:
  typedef std::vector<std::string> ButtonsStr;
  typedef std::vector<int> Buttons;
  ButtonsStr m_buttons_str;
  Buttons    m_buttons;
};

std::ostream& operator<<(std::ostream& os, const ButtonCombination& buttons);

#endif

/* EOF */
