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

#include "log_axis_event_handler.hpp"

#include <iostream>

namespace xboxdrv {

LogAxisEventHandler::LogAxisEventHandler(const std::string& format) :
  m_format(format)
{
}

void
LogAxisEventHandler::send(int value, int min, int max)
{
  std::cout << m_format << ": " << value << ":" << min << ":" << max << std::endl;
}

void
LogAxisEventHandler::update(int msec_delta)
{
}

std::string
LogAxisEventHandler::str() const
{
  return "log:" + m_format;
}

} // namespace xboxdrv

/* EOF */
