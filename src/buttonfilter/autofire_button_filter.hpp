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

#ifndef HEADER_XBOXDRV_BUTTONFILTER_AUTOFIRE_BUTTON_FILTER_HPP
#define HEADER_XBOXDRV_BUTTONFILTER_AUTOFIRE_BUTTON_FILTER_HPP

#include "button_filter.hpp"

namespace xboxdrv {

class AutofireButtonFilter : public ButtonFilter
{
public:
  static AutofireButtonFilter* from_string(const std::string& str);

public:
  /** rate: full pulse period in ms. delay: solid hold before pulsing.
      sustain: ms pressed each shot; -1 = default max(50, rate/2). */
  AutofireButtonFilter(int rate, int delay, int sustain = -1);

  void update(int msec_delta) override;
  bool filter(bool value) override;
  std::string str() const override;

private:
  enum Phase
  {
    kIdle,
    kDelay,   // physical held, still solid before autofire
    kHigh,    // pulse pressed
    kLow      // pulse released
  };

  bool m_held;
  Phase m_phase;
  int m_rate;
  int m_delay;
  int m_high;     // ms pressed each shot
  int m_low;      // ms released between shots
  int m_counter;
};

} // namespace xboxdrv

#endif

/* EOF */
