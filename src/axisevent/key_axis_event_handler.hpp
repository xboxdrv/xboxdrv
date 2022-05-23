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

#ifndef HEADER_XBOXDRV_AXISEVENT_KEY_AXIS_EVENT_HANDLER_HPP
#define HEADER_XBOXDRV_AXISEVENT_KEY_AXIS_EVENT_HANDLER_HPP

#include "axis_event.hpp"

#include <uinpp/event_sequence.hpp>

class KeyAxisEventHandler : public AxisEventHandler
{
public:
  static KeyAxisEventHandler* from_string(uinpp::MultiDevice& uinput, int slot, bool extra_devices,
                                          const std::string& str);

public:
  KeyAxisEventHandler(uinpp::MultiDevice& uinput, int slot, bool extra_devices,
                      uinpp::EventSequence up_codes,
                      uinpp::EventSequence down_codes,
                      float threshold);

  void send(int value, int min, int max) override;
  void update(int msec_delta) override;

  std::string str() const override;

private:
  int  get_zone(float value) const;

private:
  int m_old_value;

  // Array is terminated by -1
  uinpp::EventSequence m_up_codes;
  uinpp::EventSequence m_down_codes;
  float m_threshold;
};

#endif

/* EOF */
