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
#include <boost/shared_ptr.hpp>

#include "axis_filter.hpp"
#include "button_filter.hpp"
#include "xboxmsg.hpp"

class Modifier;
class Options;

typedef boost::shared_ptr<Modifier> ModifierPtr;

class Modifier
{
public:
  virtual void update(int msec_delta, XboxGenericMsg& msg) =0;
};

struct AutoFireMapping {
  static AutoFireMapping from_string(const std::string& lhs, const std::string& rhs);

  XboxButton button;
  int        frequency;
};

struct ButtonMapping {
  static ButtonMapping from_string(const std::string& lhs, const std::string& rhs);

  XboxButton lhs;
  XboxButton rhs;
  std::vector<ButtonFilterPtr> filters;
};

struct AxisMapping {
  static AxisMapping from_string(const std::string& lhs, const std::string& rhs);

  XboxAxis lhs;
  XboxAxis rhs;
  bool     invert;
  std::vector<AxisFilterPtr> filters;
};

struct RelativeAxisMapping {
  static RelativeAxisMapping from_string(const std::string& lhs, const std::string& rhs);

  XboxAxis axis;
  int      speed;
};

struct CalibrationMapping {
  static CalibrationMapping from_string(const std::string& lhs, const std::string& rhs);

  XboxAxis axis;
  int min;
  int center;
  int max;
};

struct AxisSensitivityMapping {
  static AxisSensitivityMapping from_string(const std::string& lhs, const std::string& rhs);

  XboxAxis axis;
  float sensitivity;
};

#endif

/* EOF */
