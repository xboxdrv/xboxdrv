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

#include "axisevent/rumble_axis_event_handler.hpp"

#include <iostream>

RumbleAxisEventHandler*
RumbleAxisEventHandler::from_string(const std::string& args)
{
  return new RumbleAxisEventHandler;
}

RumbleAxisEventHandler::RumbleAxisEventHandler()
{
}

void
RumbleAxisEventHandler::send(int value)
{
  std::cout << "rumble: " << value << std::endl;
}

void
RumbleAxisEventHandler::update(int msec_delta)
{
}
  
std::string
RumbleAxisEventHandler::str() const
{
  return "rumble";
}

/* EOF */
