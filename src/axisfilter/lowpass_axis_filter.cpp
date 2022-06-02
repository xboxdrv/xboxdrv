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

#include "axisfilter/lowpass_axis_filter.hpp"

#include "util/math.hpp"
#include "util/string.hpp"

namespace xboxdrv {

AxisFilterPtr
LowpassAxisFilter::from_string(std::string const& str)
{
  return AxisFilterPtr(new LowpassAxisFilter(str2float(str)));
}

LowpassAxisFilter::LowpassAxisFilter(float rate) :
  m_rate(rate),
  m_prev(0.0f),
  m_value(0.0f)
{
}

void
LowpassAxisFilter::update(int msec_delta)
{
  float rate = m_rate * static_cast<float>(msec_delta) / 1000.0f;
  m_prev = m_prev + rate * (m_value - m_prev);
}

int
LowpassAxisFilter::filter(int value, int min, int max)
{
  m_value = to_float(value, min, max);

  return from_float(m_prev, min, max);
}

std::string
LowpassAxisFilter::str() const
{
  return "lowpass";
}

} // namespace xboxdrv

/* EOF */
