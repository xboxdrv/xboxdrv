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
  UInput& m_uinput;
  int  m_slot;
  bool m_extra_devices;

public:
  ButtonEventFactory(UInput& uinput, int slot, bool extra_devices);

  ButtonEventPtr from_string(const std::string& str, const std::string& directory);

private:
  ButtonEventPtr create(ButtonEventHandler* handler);
  ButtonEventPtr create_key(int code);
  ButtonEventPtr create_key(int device_id, int code);
  ButtonEventPtr create_key();
  ButtonEventPtr create_abs(int code);
  ButtonEventPtr create_rel(int code);

private:
  ButtonEventFactory(const ButtonEventFactory&);
  ButtonEventFactory& operator=(const ButtonEventFactory&);
};

#endif

/* EOF */
