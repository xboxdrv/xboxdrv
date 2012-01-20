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

#ifndef HEADER_XBOXDRV_XBOX360_NAMES_HPP
#define HEADER_XBOXDRV_XBOX360_NAMES_HPP

class ControllerMessageDescriptor;

class Xbox360DefaultNames
{
public:
  // face button
  int btn_a;
  int btn_b;
  int btn_x;
  int btn_y;

  // option button
  int btn_start;
  int btn_guide;
  int btn_back;

  // thumb stick click
  int btn_thumb_l;
  int btn_thumb_r;

  // shoulder buttons
  int btn_lb;
  int btn_rb;

  int dpad_up;
  int dpad_down;
  int dpad_left;
  int dpad_right;

  int dpad_x;
  int dpad_y;

  // left stick
  int abs_x1;
  int abs_y1;

  // right stick
  int abs_x2;
  int abs_y2;

  // trigger
  int abs_lt;
  int abs_rt;

public:
  Xbox360DefaultNames(ControllerMessageDescriptor& msg_desc);

private:
  Xbox360DefaultNames(const Xbox360DefaultNames&);
  Xbox360DefaultNames& operator=(const Xbox360DefaultNames&);
};

#endif

/* EOF */
