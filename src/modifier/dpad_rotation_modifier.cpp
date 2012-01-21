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

#include "modifier/dpad_rotation_modifier.hpp"

#include <stdexcept>
#include <boost/lexical_cast.hpp>

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
  int degree = boost::lexical_cast<int>(value);
  degree /= 45;
  degree %= 8;
  if (degree < 0) 
    degree += 8;
    
  return new DpadRotationModifier(degree);
}

DpadRotationModifier::DpadRotationModifier(int dpad_rotation) :
  m_dpad_rotation(dpad_rotation),
  dpad_up(-1),
  dpad_down(-1),
  dpad_left(-1),
  dpad_right(-1)
{
}

void
DpadRotationModifier::init(ControllerMessageDescriptor& desc)
{
  dpad_up    = desc.key().get("dpad_up");
  dpad_down  = desc.key().get("dpad_down");
  dpad_left  = desc.key().get("dpad_left");
  dpad_right = desc.key().get("dpad_right");
}

void
DpadRotationModifier::update(int msec_delta, ControllerMessage& msg, const ControllerMessageDescriptor& desc)
{
  int up   = msg.get_key(dpad_up);
  int down = msg.get_key(dpad_down);
  int left = msg.get_key(dpad_left);
  int right= msg.get_key(dpad_right);

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
    msg.set_key(XBOX_DPAD_UP,    0);
    msg.set_key(XBOX_DPAD_DOWN,  0);
    msg.set_key(XBOX_DPAD_LEFT,  0);
    msg.set_key(XBOX_DPAD_RIGHT, 0);

    // apply the given direction
    switch(direction)
    {
      case 0:
        msg.set_key(dpad_up, 1);
        break;

      case 1:
        msg.set_key(dpad_up, 1);
        msg.set_key(dpad_right, 1);
        break;

      case 2:
        msg.set_key(dpad_right, 1);
        break;

      case 3:
        msg.set_key(dpad_right, 1);
        msg.set_key(dpad_down, 1);
        break;

      case 4:
        msg.set_key(dpad_down, 1);
        break;

      case 5:
        msg.set_key(dpad_down, 1);
        msg.set_key(dpad_left, 1);
        break;

      case 6:
        msg.set_key(dpad_left, 1);
        break;

      case 7:
        msg.set_key(dpad_up, 1);
        msg.set_key(dpad_left, 1);
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
