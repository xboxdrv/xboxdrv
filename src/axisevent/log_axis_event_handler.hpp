/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2012 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_XBOXDRV_AXISEVENT_LOG_AXIS_EVENT_HANDLER_HPP
#define HEADER_XBOXDRV_AXISEVENT_LOG_AXIS_EVENT_HANDLER_HPP

#include "axis_event.hpp"

class LogAxisEventHandler : public AxisEventHandler
{
private:
  std::string m_format;

public:
  LogAxisEventHandler(const std::string& format);

  void send(int value, int min, int max);
  void update(int msec_delta);

  std::string str() const;

private:
  LogAxisEventHandler(const LogAxisEventHandler&);
  LogAxisEventHandler& operator=(const LogAxisEventHandler&);
};

#endif

/* EOF */
