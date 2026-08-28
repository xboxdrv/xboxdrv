/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2012-2026 Ingo Ruhnke <grumbel@gmail.com>
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

#include <logmich/log.hpp>

namespace xboxdrv {

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
  // Classic Xbox pads report the dpad as four keys only. Default uinput maps
  // use gamepad.dpad_x/y → ABS_HAT0*; synthesize those axes from the keys.
  // Xbox360DefaultNames pre-registers the abs names so maps can resolve early;
  // we still drive the values every update when the keys exist.
  if (desc.key().has("dpad_up") &&
      desc.key().has("dpad_down") &&
      desc.key().has("dpad_left") &&
      desc.key().has("dpad_right"))
  {
    log_debug("CompatModifier: synthesize dpad_x/dpad_y from dpad buttons");
    m_dpad_x = desc.abs().getput("dpad_x");
    m_dpad_y = desc.abs().getput("dpad_y");

    m_dpad_up    = desc.key().get("dpad_up");
    m_dpad_down  = desc.key().get("dpad_down");
    m_dpad_left  = desc.key().get("dpad_left");
    m_dpad_right = desc.key().get("dpad_right");

    m_dpad = true;
  }

  // Combined "trigger" / rudder axis (RT − LT) for older joystick APIs that
  // expect a single Z/rudder channel instead of separate LT/RT.
  if (!desc.abs().has("trigger") &&
      desc.abs().has("lt") &&
      desc.abs().has("rt"))
  {
    log_debug("CompatModifier: synthesize trigger axis from lt/rt");
    m_abs_trigger = desc.abs().put("trigger");
    m_lt = desc.abs().get("lt");
    m_rt = desc.abs().get("rt");

    m_trigger = true;
  }
}

void
CompatModifier::update(int msec_delta, ControllerMessage& msg, ControllerMessageDescriptor const& desc)
{
  if (m_dpad)
  {
    msg.set_abs(m_dpad_x,
                (-1 * msg.get_key(m_dpad_left)) + (+1 * msg.get_key(m_dpad_right)),
                -1, 1);
    msg.set_abs(m_dpad_y,
                (-1 * msg.get_key(m_dpad_up)) + (+1 * msg.get_key(m_dpad_down)),
                -1, 1);
  }

  if (m_trigger)
  {
    // Symmetric range around 0: fully LT → negative, fully RT → positive.
    int const max_lt = msg.get_abs_max(m_lt);
    int const max_rt = msg.get_abs_max(m_rt);
    int const span = (max_lt > max_rt) ? max_lt : max_rt;
    msg.set_abs(m_abs_trigger,
                msg.get_abs(m_rt) - msg.get_abs(m_lt),
                -span, span);
  }
}

std::string
CompatModifier::str() const
{
  return "compat";
}

} // namespace xboxdrv

/* EOF */
