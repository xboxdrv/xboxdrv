/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2012 Ingo Ruhnke <grumbel@gmx.de>
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

#include "compat_modifier.hpp"

#include "log.hpp"

CompatModifier::CompatModifier() :
  m_dpad(false),
  m_dpad_x(-1),
  m_dpad_y(-1),
  m_dpad_up(-1),
  m_dpad_down(-1),
  m_dpad_left(-1),
  m_dpad_right(-1),
  m_trigger(false),
  m_abs_trigger(-1),
  m_lt(-1),
  m_rt(-1)
{
}

void
CompatModifier::init(ControllerMessageDescriptor& desc)
{
  log_tmp("CompatModifier");

#if 0
  // have a dpad, but no dpad axis
  if (!desc.abs().has(AbsName("gamepad.dpad_x")) && 
      !desc.abs().has(AbsName("gamepad.dpad_y")) &&
      desc.key().has(KeyName("gamepad.dpad_up")) &&
      desc.key().has(KeyName("gamepad.dpad_down")) &&
      desc.key().has(KeyName("gamepad.dpad_left")) &&
      desc.key().has(KeyName("gamepad.dpad_right")))
  {
    log_tmp("CompatModifier: DPAD");
    m_dpad_x = desc.abs().put("dpad_x");
    m_dpad_y = desc.abs().put("dpad_y");

    m_dpad_up    = desc.key().get("du");
    m_dpad_down  = desc.key().get("dd");
    m_dpad_left  = desc.key().get("dl");
    m_dpad_right = desc.key().get("dr");

    m_dpad = true;
  }

  // make a rudder out of both trigger
  if (!desc.abs().has("trigger") &&
      desc.abs().has("lt") &&
      desc.abs().has("rt"))
  {
    m_abs_trigger = desc.abs().put("trigger");
    m_lt = desc.abs().get("lt");
    m_rt = desc.abs().get("rt");

    m_trigger = true;
  }
#endif
}

void
CompatModifier::update(int msec_delta, ControllerMessage& msg, const ControllerMessageDescriptor& desc)
{
  if (m_dpad)
  {
    msg.set_abs(m_dpad_x, (-1 * msg.get_key(m_dpad_left)) + (+1 * msg.get_key(m_dpad_right)), -1, 1);
    msg.set_abs(m_dpad_y, (-1 * msg.get_key(m_dpad_up))   + (+1 * msg.get_key(m_dpad_down)),  -1, 1);
  }

  if (m_trigger)
  {
    msg.set_abs(m_abs_trigger, 
                msg.get_abs(m_rt) - msg.get_abs(m_lt),
                -msg.get_abs_max(m_lt),
                msg.get_abs_max(m_lt));
  }
}

std::string
CompatModifier::str() const
{
  return "compat";
}

/* EOF */
