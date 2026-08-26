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

#include "buttonfilter/autofire_button_filter.hpp"

#include <algorithm>
#include <sstream>

#include <strut/split.hpp>

#include "util/string.hpp"

namespace xboxdrv {

AutofireButtonFilter*
AutofireButtonFilter::from_string(std::string const& str)
{
  int rate    = 50;
  int delay   = 0;
  int sustain = -1; // default: max(50, rate/2)

  auto tokens = strut::split(str, ':');
  int idx = 0;
  for(auto t = tokens.begin(); t != tokens.end(); ++t, ++idx)
  {
    switch(idx)
    {
      case 0: rate    = str2int(*t); break;
      case 1: delay   = str2int(*t); break;
      case 2: sustain = str2int(*t); break;
      default:
        throw std::runtime_error(
          "autofire filter: too many arguments (RATE, RATE:DELAY, or RATE:DELAY:SUSTAIN)");
    }
  }

  if (rate <= 0)
  {
    throw std::runtime_error("autofire filter: RATE must be > 0");
  }

  return new AutofireButtonFilter(rate, delay, sustain);
}

AutofireButtonFilter::AutofireButtonFilter(int rate, int delay, int sustain) :
  m_held(false),
  m_phase(kIdle),
  m_rate(std::max(1, rate)),
  m_delay(std::max(0, delay)),
  m_high(0),
  m_low(0),
  m_counter(0)
{
  if (sustain < 0)
  {
    // Default: long enough for games to see the press; not a one-tick pulse.
    m_high = std::max(50, m_rate / 2);
  }
  else
  {
    m_high = sustain;
  }

  if (m_high < 1)
  {
    m_high = 1;
  }
  if (m_high >= m_rate)
  {
    // Need a non-zero low phase so the output actually toggles.
    m_high = std::max(1, m_rate - 1);
  }
  m_low = m_rate - m_high;
}

void
AutofireButtonFilter::update(int msec_delta)
{
  if (!m_held || m_phase == kIdle)
  {
    return;
  }

  m_counter += msec_delta;

  switch (m_phase)
  {
    case kDelay:
      if (m_counter >= m_delay)
      {
        // Release before the pulse train so the initial solid hold does not
        // merge with the first HIGH into one long press.
        m_phase = kLow;
        m_counter = 0;
      }
      break;

    case kHigh:
      if (m_counter >= m_high)
      {
        m_phase = kLow;
        m_counter = 0;
      }
      break;

    case kLow:
      if (m_counter >= m_low)
      {
        m_phase = kHigh;
        m_counter = 0;
      }
      break;

    case kIdle:
      break;
  }
}

bool
AutofireButtonFilter::filter(bool value)
{
  if (!value)
  {
    m_held = false;
    m_phase = kIdle;
    m_counter = 0;
    return false;
  }

  if (!m_held)
  {
    // rising edge
    m_held = true;
    m_counter = 0;
    m_phase = (m_delay > 0) ? kDelay : kHigh;
  }

  switch (m_phase)
  {
    case kDelay:
    case kHigh:
      return true;
    case kLow:
      return false;
    case kIdle:
    default:
      return true;
  }
}

std::string
AutofireButtonFilter::str() const
{
  std::ostringstream out;
  out << "auto:" << m_rate << ":" << m_delay << ":" << m_high;
  return out.str();
}

} // namespace xboxdrv

/* EOF */
