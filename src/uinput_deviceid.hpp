/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2010 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_XBOXDRV_UINPUT_DEVICEID_HPP
#define HEADER_XBOXDRV_UINPUT_DEVICEID_HPP

#include <boost/lexical_cast.hpp>

enum {
  DEVICEID_INVALID  = -4,
  DEVICEID_KEYBOARD = -3,
  DEVICEID_MOUSE    = -2,
  DEVICEID_AUTO     = -1,
  DEVICEID_JOYSTICK =  0
};

struct UIEvent 
{
  static UIEvent create(int device_id, int code) 
  {
    UIEvent ev;
    ev.device_id = device_id;
    ev.code      = code;
    return ev;
  }

  static UIEvent invalid()
  {
    UIEvent ev;
    ev.device_id = DEVICEID_INVALID;
    ev.code      = -1;
    return ev;    
  }

  bool is_valid() const {
    return 
      device_id == DEVICEID_INVALID || 
      code == -1;
  }

  int device_id;
  int code;
};

/** Takes "1-BTN_A" splits it into "1", "BTN_A" */
inline void split_event_name(const std::string& str, std::string* event_str, int* device_id)
{
  std::string::size_type p = str.find('-');
  if (p == std::string::npos)
  {
    *event_str = str;
    *device_id = DEVICEID_AUTO;
  }
  else
  {
    *event_str = str.substr(p+1);
    std::string device = str.substr(0, p);

    if (device == "auto")
    {
      *device_id = DEVICEID_AUTO;
    }
    else if (device == "mouse")
    {
      *device_id = DEVICEID_MOUSE;
    }
    else if (device == "keyboard")
    {
      *device_id = DEVICEID_KEYBOARD;
    }
    else
    {
      *device_id = boost::lexical_cast<int>(device);
    }
  }
}

inline void split_string_at(const std::string& str, char c, std::string* lhs, std::string* rhs)
{
  std::string::size_type p = str.find(c);
  if (p == std::string::npos)
  {
    *lhs = str;
  }
  else
  {
    *lhs = str.substr(0, p);
    *rhs = str.substr(p+1);
  }
}

#endif

/* EOF */
