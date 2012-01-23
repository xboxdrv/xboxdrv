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

#ifndef HEADER_XBOXDRV_AXISEVENT_RUMBLE_AXIS_EVENT_HANDLER_HPP
#define HEADER_XBOXDRV_AXISEVENT_RUMBLE_AXIS_EVENT_HANDLER_HPP

#include "axis_event.hpp"

class RumbleAxisEventHandler : public AxisEventHandler
{
private:
public:
  static RumbleAxisEventHandler* from_string(const std::string& args);

public:
  RumbleAxisEventHandler();

  void send(int value);
  void update(int msec_delta);
  
  std::string str() const;

private:
  RumbleAxisEventHandler(const RumbleAxisEventHandler&);
  RumbleAxisEventHandler& operator=(const RumbleAxisEventHandler&);
};

#endif

/* EOF */
