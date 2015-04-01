/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2012 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_XBOXDRV_BUTTONEVENT_LOG_BUTTON_EVENT_HANDLER_HPP
#define HEADER_XBOXDRV_BUTTONEVENT_LOG_BUTTON_EVENT_HANDLER_HPP

#include "button_event.hpp"

class LogButtonEventHandler : public ButtonEventHandler
{
private:
  std::string m_format;

public:
  LogButtonEventHandler(const std::string& format);

  void send(bool value);
  void update(int msec_delta);
  std::string str() const;

private:
  LogButtonEventHandler(const LogButtonEventHandler&);
  LogButtonEventHandler& operator=(const LogButtonEventHandler&);
};

#endif

/* EOF */
