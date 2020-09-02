/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmail.com>
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

#include "modifier/dpad_rotation_modifier.hpp"

#include <stdexcept>
#include <sstream>

#include "util/string.hpp"

#include "controller_config.hpp"

DpadRotationModifier*
DpadRotationModifier::from_string(const std::vector<std::string>& args)
{
  if (args.size() != 1)
  {
    throw std::runtime_error("DpadRotationModifier expects exactly one argument");
  }
  else
  {
    return DpadRotationModifier::from_string(args[0]);
  }
}

DpadRotationModifier*
DpadRotationModifier::from_string(const std::string& value)
{
  int degree = str2int(value);
  degree /= 45;
  degree %= 8;
  if (degree < 0)
    degree += 8;

  return new DpadRotationModifier(degree);
}

DpadRotationModifier::DpadRotationModifier(int dpad_rotation) :
  m_dpad_rotation(dpad_rotation),

  m_dpad_up("dpad_up"),
  m_dpad_down("dpad_down"),
  m_dpad_left("dpad_left"),
  m_dpad_right("dpad_right"),

  m_dpad_up_out("dpad_up"),
  m_dpad_down_out("dpad_down"),
  m_dpad_left_out("dpad_left"),
  m_dpad_right_out("dpad_right")
{
}

void
DpadRotationModifier::init(ControllerMessageDescriptor& desc)
{
  m_dpad_up.init(desc);
  m_dpad_down.init(desc);
  m_dpad_left.init(desc);
  m_dpad_right.init(desc);
}

void
DpadRotationModifier::update(int msec_delta, ControllerMessage& msg, const ControllerMessageDescriptor& desc)
{
  int up   = m_dpad_up.get(msg);
  int down = m_dpad_down.get(msg);
  int left = m_dpad_left.get(msg);
  int right= m_dpad_right.get(msg);

  // -1: not pressed, 0: up, 1: up/right, ...
  int direction = -1;
  if (up && !down && !left && !right)
  {
    direction = 0;
  }
  else if (up && !down && !left && right)
  {
    direction = 1;
  }
  else if (!up && !down && !left && right)
  {
    direction = 2;
  }
  else if (!up && down && !left && right)
  {
    direction = 3;
  }
  else if (!up && down && !left && !right)
  {
    direction = 4;
  }
  else if (!up && down && left && !right)
  {
    direction = 5;
  }
  else if (!up && !down && left && !right)
  {
    direction = 6;
  }
  else if (up && !down && left && !right)
  {
    direction = 7;
  }

  if (direction != -1)
  {
    direction += m_dpad_rotation;
    direction %= 8;
    if (direction < 0)
      direction += 8;

    // set everything to zero
    m_dpad_up_out.set(msg, 0);
    m_dpad_down_out.set(msg, 0);
    m_dpad_left_out.set(msg, 0);
    m_dpad_right_out.set(msg, 0);

    // apply the given direction
    switch(direction)
    {
      case 0:
        m_dpad_up_out.set(msg, 1);
        break;

      case 1:
        m_dpad_up_out.set(msg, 1);
        m_dpad_right_out.set(msg, 1);
        break;

      case 2:
        m_dpad_right_out.set(msg, 1);
        break;

      case 3:
        m_dpad_right_out.set(msg, 1);
        m_dpad_down_out.set(msg, 1);
        break;

      case 4:
        m_dpad_down_out.set(msg, 1);
        break;

      case 5:
        m_dpad_down_out.set(msg, 1);
        m_dpad_left_out.set(msg, 1);
        break;

      case 6:
        m_dpad_left_out.set(msg, 1);
        break;

      case 7:
        m_dpad_up_out.set(msg, 1);
        m_dpad_left_out.set(msg, 1);
        break;
    }
  }
}

std::string
DpadRotationModifier::str() const
{
  std::ostringstream out;
  out << "dpad-rotation:" << m_dpad_rotation;
  return out.str();
}

/* EOF */
