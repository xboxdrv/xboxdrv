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

#ifndef HEADER_XBOXDRV_CONTROLLER_MATCH_RULE_HPP
#define HEADER_XBOXDRV_CONTROLLER_MATCH_RULE_HPP

#include <string>

class ControllerMatchRule
{
public:
  enum { 
    kMatchEverything,
    kMatchUSBId, 
    kMatchUSBPath, 
    kMatchEvdevPath
  } m_type;

  int m_bus;
  int m_dev;
  
  int m_vendor;
  int m_product;

  std::string m_path;

  ControllerMatchRule() :
    m_type(kMatchEverything),
    m_bus(),
    m_dev(),
    m_vendor(),
    m_product(),
    m_path()
  {}

  static ControllerMatchRule match_usb_id(int vendor, int product);
  static ControllerMatchRule match_usb_path(int bus, int dev);
  static ControllerMatchRule match_evdev_path(const std::string& path);
};

#endif

/* EOF */
