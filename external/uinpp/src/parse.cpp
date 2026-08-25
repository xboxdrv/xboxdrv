// uinpp - Linux uinput library for C++
// Copyright (C) 2008-2022 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "parse.hpp"

#include <limits>
#include <sstream>
#include <vector>

#include <fmt/format.h>
#include <strut/split.hpp>

#include "event.hpp"

namespace uinpp {

namespace {

int hexstr2int(std::string const& str)
{
  unsigned int value = 0;
  if (sscanf(str.c_str(), "%x", &value) == 1)
  {
    return value;
  }
  else if (sscanf(str.c_str(), "0x%x", &value) == 1)
  {
    return value;
  }
  else
  {
    std::ostringstream err;
    err << "couldn't convert '" << str << "' to int";
    throw std::runtime_error(err.str());
  }
}

uint16_t hexstr2uint16(std::string const& str)
{
  return static_cast<uint16_t>(hexstr2int(str));
}

} // namespace

input_id parse_input_id(std::string const& str)
{
  struct input_id usbid;

  // default values
  usbid.bustype = BUS_USB;
  usbid.vendor  = 0;
  usbid.product = 0;
  usbid.version = 0;

  // split string at ':'
  std::vector<std::string> args = strut::split(str, ':');

  if (args.size() == 2)
  { // VENDOR:PRODUCT
    usbid.vendor  = hexstr2uint16(args[0]);
    usbid.product = hexstr2uint16(args[1]);
  }
  else if (args.size() == 3)
  { // VENDOR:PRODUCT:VERSION
    usbid.vendor  = hexstr2uint16(args[0]);
    usbid.product = hexstr2uint16(args[1]);
    usbid.version = hexstr2uint16(args[2]);
  }
  else if (args.size() == 4)
  { // VENDOR:PRODUCT:VERSION:BUS
    usbid.vendor  = hexstr2uint16(args[0]);
    usbid.product = hexstr2uint16(args[1]);
    usbid.version = hexstr2uint16(args[2]);
    usbid.bustype = hexstr2uint16(args[3]);
  }
  else
  {
    throw std::runtime_error("incorrect number of arguments");
  }

  return usbid;
}

uint32_t
parse_device_id(std::string const& str)
{
  // FIXME: insert magic to resolve symbolic names, merge with same code in set_device_name
  std::string::size_type p = str.find('.');

  uint16_t device_id;
  uint16_t slot_id;

  if (p == std::string::npos)
  {
    device_id = str2deviceid(str.substr());
    slot_id   = SLOTID_AUTO;
  }
  else if (p == 0)
  {
    device_id = DEVICEID_AUTO;
    slot_id   = str2slotid(str.substr(p+1));
  }
  else
  {
    device_id = str2deviceid(str.substr(0, p));
    slot_id   = str2slotid(str.substr(p+1));
  }

  return create_device_id(slot_id, device_id);
}

uint16_t str2deviceid(std::string const& device)
{
  if (device == "auto" || device.empty())
  {
    return DEVICEID_AUTO;
  }
  else if (device == "mouse")
  {
    return DEVICEID_MOUSE;
  }
  else if (device == "keyboard" || device == "key")
  {
    return DEVICEID_KEYBOARD;
  }
  else if (device == "joystick" || device == "joy")
  {
    return DEVICEID_JOYSTICK;
  }
  else
  {
    int result = std::stoi(device);
    if (result < 0 || result > std::numeric_limits<uint16_t>::max())
    {
      throw std::runtime_error(fmt::format("str2deviceid(): out of range: '{}'", device));
    }
    else
    {
      return static_cast<uint16_t>(result);
    }
  }
}

uint16_t str2slotid(std::string const& slot)
{
  if (slot == "auto" || slot.empty())
  {
    return SLOTID_AUTO;
  }
  else
  {
    int result = std::stoi(slot);
    if (result < 0 || result > std::numeric_limits<uint16_t>::max())
    {
      throw std::runtime_error(fmt::format("str2deviceid(): out of range: '{}'", slot));
    }
    else
    {
      return static_cast<uint16_t>(result);
    }
  }
}

void split_event_name(std::string const& str, std::string* event_str, int* slot_id, int* device_id)
{
  std::string::size_type p = str.find('@');
  if (p == std::string::npos)
  {
    *event_str = str;
    *slot_id   = SLOTID_AUTO;
    *device_id = DEVICEID_AUTO;
  }
  else
  {
    *event_str = str.substr(0, p);
    std::string device = str.substr(p+1);

    p = device.find(".");

    if (p == std::string::npos)
    {
      *slot_id   = SLOTID_AUTO;
      *device_id = str2deviceid(device);
    }
    else
    {
      *slot_id   = str2slotid(device.substr(p+1));
      *device_id = str2deviceid(device.substr(0, p));
    }
  }
}

} // namespace uinpp

/* EOF */
