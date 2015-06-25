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

#ifndef HEADER_XBOXDRV_EVDEV_ABSMAP_HPP
#define HEADER_XBOXDRV_EVDEV_ABSMAP_HPP

#include <map>

#include "xboxmsg.hpp"

struct XboxGenericMsg;

class EvdevAbsMap
{
public:
  EvdevAbsMap();

  void process(XboxGenericMsg& msg, int code, int value, int min, int max) const;

  void bind_plus(int code, XboxAxis axis);
  void bind_minus(int code, XboxAxis axis);
  void bind_both(int code, XboxAxis axis);

  void clear();

private:
  std::map<int, XboxAxis> m_plus_map;
  std::map<int, XboxAxis> m_minus_map;
  std::map<int, XboxAxis> m_both_map;
};

#endif

/* EOF */
