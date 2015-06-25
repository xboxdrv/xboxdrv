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
#include <boost/lexical_cast.hpp>

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
  int degree = boost::lexical_cast<int>(value);
  degree /= 45;
  degree %= 8;
  if (degree < 0)
    degree += 8;

  return new DpadRotationModifier(degree);
}

DpadRotationModifier::DpadRotationModifier(int dpad_rotation) :
  m_dpad_rotation(dpad_rotation)
{
}

void
DpadRotationModifier::update(int msec_delta, XboxGenericMsg& msg)
{
  int up    = get_button(msg, XBOX_DPAD_UP);
  int down  = get_button(msg, XBOX_DPAD_DOWN);
  int left  = get_button(msg, XBOX_DPAD_LEFT);
  int right = get_button(msg, XBOX_DPAD_RIGHT);

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
    set_button(msg, XBOX_DPAD_UP,    0);
    set_button(msg, XBOX_DPAD_DOWN,  0);
    set_button(msg, XBOX_DPAD_LEFT,  0);
    set_button(msg, XBOX_DPAD_RIGHT, 0);

    // apply the given direction
    switch(direction)
    {
      case 0:
        set_button(msg, XBOX_DPAD_UP, 1);
        break;

      case 1:
        set_button(msg, XBOX_DPAD_UP, 1);
        set_button(msg, XBOX_DPAD_RIGHT, 1);
        break;

      case 2:
        set_button(msg, XBOX_DPAD_RIGHT, 1);
        break;

      case 3:
        set_button(msg, XBOX_DPAD_RIGHT, 1);
        set_button(msg, XBOX_DPAD_DOWN, 1);
        break;

      case 4:
        set_button(msg, XBOX_DPAD_DOWN, 1);
        break;

      case 5:
        set_button(msg, XBOX_DPAD_DOWN, 1);
        set_button(msg, XBOX_DPAD_LEFT, 1);
        break;

      case 6:
        set_button(msg, XBOX_DPAD_LEFT, 1);
        break;

      case 7:
        set_button(msg, XBOX_DPAD_UP, 1);
        set_button(msg, XBOX_DPAD_LEFT, 1);
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
