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

#ifndef HEADER_XBOXDRV_AXISEVENT_REL_REPEAT_AXIS_EVENT_HANDLER_HPP
#define HEADER_XBOXDRV_AXISEVENT_REL_REPEAT_AXIS_EVENT_HANDLER_HPP

#include "axis_event.hpp"

#include <uinpp/event_emitter.hpp>

namespace xboxdrv {

class RelRepeatAxisEventHandler : public AxisEventHandler
{
public:
  static RelRepeatAxisEventHandler* from_string(uinpp::MultiDevice& uinput, int slot, bool extra_devices,
                                                const std::string& str);

public:
  RelRepeatAxisEventHandler(uinpp::MultiDevice& uinput, int slot, bool extra_devices,
                            const uinpp::Event& code, int value, float repeat);

  void send(int value, int min, int max) override;
  void update(int msec_delta) override;

  std::string str() const override;

private:
  uinpp::Event m_code;
  int     m_value;
  float   m_repeat;

  float   m_stick_value;
  float   m_timer;

  uinpp::EventEmitter* m_rel_emitter;

public:
  RelRepeatAxisEventHandler(const RelRepeatAxisEventHandler&) = delete;
  RelRepeatAxisEventHandler& operator=(const RelRepeatAxisEventHandler&) = delete;
};

} // namespace xboxdrv

#endif

/* EOF */
