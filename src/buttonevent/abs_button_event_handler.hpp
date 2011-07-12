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

#ifndef HEADER_XBOXDRV_BUTTONEVENT_ABS_BUTTON_EVENT_HANDLER_HPP
#define HEADER_XBOXDRV_BUTTONEVENT_ABS_BUTTON_EVENT_HANDLER_HPP

#include "button_event.hpp"

#include "ui_event_emitter.hpp"

class AbsButtonEventHandler : public ButtonEventHandler
{
public:
  static AbsButtonEventHandler* from_string(const std::string& str);

public:
  AbsButtonEventHandler(int code);

  void init(UInput& uinput, int slot, bool extra_devices);
  void send(UInput& uinput, bool value);
  void update(UInput& uinput, int msec_delta) {}

  std::string str() const;

private:
  UIEvent m_code;
  int m_value;

  UIEventEmitterPtr m_abs_emitter;
};

#endif

/* EOF */
