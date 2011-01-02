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

#include <stdint.h>

#include "modifier/deadzone_modifier.hpp"

namespace {

int clamp(int lhs, int rhs, int v)
{
  return std::max(lhs, std::min(v, rhs));
}

int16_t scale_deadzone(int16_t value, const int deadzone)
{
  float rv = value;
  if (value < -deadzone) {
    const float scale = 32768 / (32768 - deadzone);
    rv += deadzone;
    rv *= scale;
    rv -= 0.5;
  } else if (value > deadzone) {
    const float scale = 32767 / (32767 - deadzone);
    rv -= deadzone;
    rv *= scale;
    rv += 0.5;
  } else {
    return 0;
  }
  return clamp(-32768, static_cast<int>(rv), 32767);
}

uint8_t scale_trigger_deadzone(uint8_t value, int deadzone)
{
  const float scale = 255 / (255 - deadzone);
    if (value <= deadzone) {
        return 0;
    } else {
        value -= deadzone;
        float rv = value * scale;
        return clamp(0, static_cast<int>(rv+0.5), 255);
    }
}

} // namespace

DeadzoneModifier::DeadzoneModifier(int deadzone, int deadzone_trigger) :
  m_deadzone(deadzone),
  m_deadzone_trigger(deadzone_trigger)
{
}

void
DeadzoneModifier::update(int msec_delta, XboxGenericMsg& msg)
{
  switch (msg.type)
  {
    case XBOX_MSG_XBOX:
      msg.xbox.x1 = scale_deadzone(msg.xbox.x1, m_deadzone);
      msg.xbox.y1 = scale_deadzone(msg.xbox.y1, m_deadzone);
      msg.xbox.x2 = scale_deadzone(msg.xbox.x2, m_deadzone);
      msg.xbox.y2 = scale_deadzone(msg.xbox.y2, m_deadzone);
      msg.xbox.lt = scale_trigger_deadzone(msg.xbox.lt, m_deadzone_trigger);
      msg.xbox.rt = scale_trigger_deadzone(msg.xbox.rt, m_deadzone_trigger);
      break;

    case XBOX_MSG_XBOX360:
      msg.xbox360.x1 = scale_deadzone(msg.xbox360.x1, m_deadzone);
      msg.xbox360.y1 = scale_deadzone(msg.xbox360.y1, m_deadzone);
      msg.xbox360.x2 = scale_deadzone(msg.xbox360.x2, m_deadzone);
      msg.xbox360.y2 = scale_deadzone(msg.xbox360.y2, m_deadzone);
      msg.xbox360.lt = scale_trigger_deadzone(msg.xbox360.lt, m_deadzone_trigger);
      msg.xbox360.rt = scale_trigger_deadzone(msg.xbox360.rt, m_deadzone_trigger);
      break;
  }
}

/* EOF */
