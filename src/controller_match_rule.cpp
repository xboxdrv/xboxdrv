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

#include "controller_match_rule.hpp"

#include <assert.h>

bool
ControllerMatchRule::match(int vendor, int product,
                           int bus, int dev) const
{
  switch(m_type)
  {
    case kMatchEverything:
      return true;

    case kMatchUSBId:
      return (vendor == m_vendor && product == m_product);

    case kMatchUSBPath:
      return (bus == m_bus && dev == m_dev);

    case kMatchEvdevPath:
      assert(!"not implemented");
      return false;
      
    default:
      assert(!"never reached");
      return false;
  }
}

ControllerMatchRule
ControllerMatchRule::match_usb_id(int vendor, int product)
{
  ControllerMatchRule rule;
  rule.m_type = kMatchUSBId;
  rule.m_vendor = vendor;
  rule.m_product = product;
  return rule;
}

ControllerMatchRule
ControllerMatchRule::match_usb_path(int bus, int dev)
{
  ControllerMatchRule rule;
  rule.m_type = kMatchUSBPath;
  rule.m_bus  = bus;
  rule.m_dev = dev;
  return rule;
}

ControllerMatchRule 
ControllerMatchRule::match_evdev_path(const std::string& path)
{
  ControllerMatchRule rule;
  rule.m_type = kMatchEvdevPath;
  rule.m_path = path;
  return rule;
}

/* EOF */
