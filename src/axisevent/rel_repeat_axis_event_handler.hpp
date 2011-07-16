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

#ifndef HEADER_XBOXDRV_AXISEVENT_REL_REPEAT_AXIS_EVENT_HANDLER_HPP
#define HEADER_XBOXDRV_AXISEVENT_REL_REPEAT_AXIS_EVENT_HANDLER_HPP

#include "axis_event.hpp"

#include "ui_event_emitter.hpp"

class RelRepeatAxisEventHandler : public AxisEventHandler
{
public:
  static RelRepeatAxisEventHandler* from_string(const std::string& str);

public:
  RelRepeatAxisEventHandler(const UIEvent& code, int value, int repeat);

  void init(UInput& uinput, int slot, bool extra_devices);
  void send(UInput& uinput, int value);
  void update(UInput& uinput, int msec_delta);

  std::string str() const;

private:
  UIEvent m_code;
  int     m_value;
  float   m_repeat;

  float   m_stick_value;
  int     m_timer;

  UIEventEmitterPtr m_rel_emitter;
};

#endif

/* EOF */
