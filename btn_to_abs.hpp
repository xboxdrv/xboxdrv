/* 
**  Xbox/Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_BTN_TO_ABS_HPP
#define HEADER_BTN_TO_ABS_HPP

#include "control.hpp"

/** */
class BtnToAbs : public Control
{
private:
  int target_value;

public:
  BtnToAbs();
  std::string get_name() const { return "BtnToAbs"; }
  void on_btn(BtnPortOut* port);
  void update(float delta);
private:
  BtnToAbs (const BtnToAbs&);
  BtnToAbs& operator= (const BtnToAbs&);
};

#endif

/* EOF */
