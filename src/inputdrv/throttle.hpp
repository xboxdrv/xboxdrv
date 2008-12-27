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

#ifndef HEADER_THROTTLE_HPP
#define HEADER_THROTTLE_HPP

#include "control.hpp"

/** */
class Throttle : public Control
{
private:
  int value;
  
public:
  Throttle();
  std::string get_name() const { return "Throttle"; } 

  void update(float delta);
  void on_abs(AbsPortOut* port);

private:
  Throttle (const Throttle&);
  Throttle& operator= (const Throttle&);
};

#endif

/* EOF */
