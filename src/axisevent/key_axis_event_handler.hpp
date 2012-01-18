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

#ifndef HEADER_XBOXDRV_AXISEVENT_KEY_AXIS_EVENT_HANDLER_HPP
#define HEADER_XBOXDRV_AXISEVENT_KEY_AXIS_EVENT_HANDLER_HPP

#include "axis_event.hpp"

#include "ui_event_sequence.hpp"

class KeyAxisEventHandler : public AxisEventHandler
{
public:
  static KeyAxisEventHandler* from_string(UInput& uinput, int slot, bool extra_devices,
                                          const std::string& str);
  
public:
  KeyAxisEventHandler(UInput& uinput, int slot, bool extra_devices);

  void send(int value);
  void update(int msec_delta);

  std::string str() const;

private:
  void send_up(int value);
  void send_down(int value);
  int  get_zone(int value) const;
  
private:
  int m_old_value;

  // Array is terminated by -1
  UIEventSequence m_up_codes;
  UIEventSequence m_down_codes;
  int m_threshold;
};

#endif

/* EOF */
