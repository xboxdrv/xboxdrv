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

#include "log_button_event_handler.hpp"

#include <iostream>

LogButtonEventHandler::LogButtonEventHandler(const std::string& format) :
  m_format(format)
{
}

void
LogButtonEventHandler::send(bool value)
{
  std::cout << m_format << ":" << value << std::endl;
}

void
LogButtonEventHandler::update(int msec_delta)
{
}

std::string
LogButtonEventHandler::str() const
{
  return "log:" + m_format;
}

/* EOF */
