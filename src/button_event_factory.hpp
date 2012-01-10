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

#ifndef HEADER_XBOXDRV_BUTTON_EVENT_FACTORY_HPP
#define HEADER_XBOXDRV_BUTTON_EVENT_FACTORY_HPP

#include "button_event.hpp"

class ButtonEventFactory
{
private:
public:
  ButtonEventFactory();

  static ButtonEventPtr create(ButtonEventHandler* handler);
  static ButtonEventPtr create_key(int code);
  static ButtonEventPtr create_key(int device_id, int code);
  static ButtonEventPtr create_key();
  static ButtonEventPtr create_abs(int code);
  static ButtonEventPtr create_rel(int code);
  static ButtonEventPtr from_string(const std::string& str, const std::string& directory);

private:
  ButtonEventFactory(const ButtonEventFactory&);
  ButtonEventFactory& operator=(const ButtonEventFactory&);
};

#endif

/* EOF */
