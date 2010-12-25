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

#ifndef HEADER_XBOXDRV_AXIS_EVENT_HPP
#define HEADER_XBOXDRV_AXIS_EVENT_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "uinput_deviceid.hpp"

class uInput;
class AxisEvent;

typedef boost::shared_ptr<AxisEvent> AxisEventPtr;

class AxisEvent
{
public:
  static const int MAX_MODIFIER = 4;

  static AxisEventPtr invalid();
  static AxisEventPtr create_abs(int device_id, int code, int min, int max, int fuzz, int flat);
  static AxisEventPtr create_rel(int device_id, int code, int repeat = 10, float value = 5);

  static AxisEventPtr create_key();
  static AxisEventPtr create_rel();
  static AxisEventPtr create_abs();

  /** If an AxisEvent gets created the user has to set min/max with set_axis_range() */ 
  static AxisEventPtr from_string(const std::string& str);

private:
  static AxisEventPtr abs_from_string(const std::string& str);
  static AxisEventPtr rel_from_string(const std::string& str);
  static AxisEventPtr key_from_string(const std::string& str);

public:
  AxisEvent();

  void set_axis_range(int min, int max);

  void init(uInput& uinput) const;
  void send(uInput& uinput, int old_value, int value) const;

  std::string str() const;

private:
  /** EV_KEY, EV_ABS, EV_REL */
  int type;

  union {
    struct {
      UIEvent code;
      float value; // FIXME: Why is this float?
      int   repeat;
    } rel;

    struct {
      UIEvent code;
      int min;
      int max;
      int fuzz;
      int flat;
    } abs;

    struct {
      // Array is terminated by -1
      UIEvent up_codes[MAX_MODIFIER+1];
      UIEvent down_codes[MAX_MODIFIER+1];
      int threshold;
    } key;
  };
};

#endif

/* EOF */
