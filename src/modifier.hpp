/* 
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_MODIFIER_HPP
#define HEADER_MODIFIER_HPP

#include <stdlib.h>
#include <string>
#include <vector>
#include "xboxmsg.hpp"

struct ButtonMapping {
  static ButtonMapping from_string(const std::string& str);

  XboxButton lhs;
  XboxButton rhs;
};

struct AxisMapping {
  static AxisMapping from_string(const std::string& str);

  XboxAxis lhs;
  XboxAxis rhs;
  bool     invert;
};

struct AutoFireMapping {
  static AutoFireMapping from_string(const std::string&);

  XboxButton button;
  int        frequency;
};

struct RelativeAxisMapping {
  static RelativeAxisMapping from_string(const std::string&);

  XboxAxis axis;
  int      speed;
};

struct CalibrationMapping {
  static CalibrationMapping from_string(const std::string&);

  XboxAxis axis;
  int min;
  int center;
  int max;
};


class RelativeAxisModifier
{
private:
  std::vector<RelativeAxisMapping> relative_axis_map;
  std::vector<int> axis_state;

public:
  RelativeAxisModifier(const std::vector<RelativeAxisMapping>& relative_axis_map) 
    : relative_axis_map(relative_axis_map)
  {
    for(size_t i = 0; i < relative_axis_map.size(); ++i)
      {
        axis_state.push_back(0);
      }
  }

  void update(int msec_delta, XboxGenericMsg& msg)
  {
    for(size_t i = 0; i < relative_axis_map.size(); ++i)
      {
        int value = get_axis(msg, relative_axis_map[i].axis);
        if (abs(value) > 4000 ) // FIXME: add proper deadzone handling
          {
            axis_state[i] += ((relative_axis_map[i].speed * value) / 32768) * msec_delta / 1000;
            if (axis_state[i] < -32768)
              axis_state[i] = -32768;
            else if (axis_state[i] > 32767)
              axis_state[i] = 32767;

            set_axis(msg, relative_axis_map[i].axis, axis_state[i]);
          }
        else
          {
            set_axis(msg, relative_axis_map[i].axis, axis_state[i]);
          }
      }
  }
};

class AutoFireModifier
{
private:
  std::vector<AutoFireMapping> autofire_map;
  std::vector<int> button_timer;

public:
  AutoFireModifier(const std::vector<AutoFireMapping>& autofire_map)
    : autofire_map(autofire_map)
  {
    for(std::vector<AutoFireMapping>::const_iterator i = autofire_map.begin(); i != autofire_map.end(); ++i)
      {
        button_timer.push_back(0.0f);
      }
  }

  void update(int msec_delta, XboxGenericMsg& msg)
  {
    for(size_t i = 0; i < autofire_map.size(); ++i)
      {
        if (get_button(msg, autofire_map[i].button))
          {
            button_timer[i] += msec_delta;

            if (button_timer[i] > autofire_map[i].frequency)
              {
                set_button(msg, autofire_map[i].button, 1);
                button_timer[i] = 0.0f; // FIXME: we ignoring the passed time
              }
            else if (button_timer[i] > autofire_map[i].frequency/2)
              {
                set_button(msg, autofire_map[i].button, 0);
              }
            else
              {
                set_button(msg, autofire_map[i].button, 1);
              }
          }
        else
          {
            button_timer[i] = 0;
          }
      }
  }
};

void apply_button_map(XboxGenericMsg& msg, std::vector<ButtonMapping>& lst);
void apply_axis_map(XboxGenericMsg& msg, std::vector<AxisMapping>& lst);
void apply_calibration_map(XboxGenericMsg& msg, std::vector<CalibrationMapping>& lst);

#endif

/* EOF */
