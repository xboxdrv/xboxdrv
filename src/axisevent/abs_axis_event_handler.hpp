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

#ifndef HEADER_XBOXDRV_AXISEVENT_ABS_AXIS_EVENT_HANDLER_HPP
#define HEADER_XBOXDRV_AXISEVENT_ABS_AXIS_EVENT_HANDLER_HPP

#include "axis_event.hpp"

class AbsAxisEventHandler : public AxisEventHandler
{
public:
  static AbsAxisEventHandler* from_string(const std::string& str);

public:
  AbsAxisEventHandler();
  AbsAxisEventHandler(const UIEvent& code, int min, int max, int fuzz, int flat);

  void init(UInput& uinput, int slot, bool extra_devices);
  void send(UInput& uinput, int value);
  void update(UInput& uinput, int msec_delta);

  std::string str() const;

private:
  UIEvent m_code;
  int m_fuzz;
  int m_flat;
};

#endif

/* EOF */
