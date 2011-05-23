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

#ifndef HEADER_XBOXDRV_CONTROLLER_MESSAGE_HPP
#define HEADER_XBOXDRV_CONTROLLER_MESSAGE_HPP

#include <linux/input.h>

class ControllerMessage
{
private:
  // FIXME: not the most compact representation
  bool m_button[KEY_CNT];
  int m_axis[ABS_CNT];
  int m_rel[REL_CNT];

public:
  ControllerMessage();

  void set_button(int button, bool value);
  int  get_button(int button) const;

  void set_axis(int axis, int value);
  int  get_axis(int axis) const;

  void set_rel(int rel, int value);
  int  get_rel(int rel) const;

  void clear();
};

#endif

/* EOF */
