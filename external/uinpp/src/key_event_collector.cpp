// uinpp - Linux uinput library for C++
// Copyright (C) 2008-2022 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "key_event_collector.hpp"

#include <cassert>

#include <logmich/log.hpp>

#include "multi_device.hpp"

namespace uinpp {

KeyEventCollector::KeyEventCollector(MultiDevice& uinput, uint32_t device_id, int type, int code) :
  EventCollector(uinput, device_id, type, code),
  m_emitters(),
  m_value(0)
{
}

EventEmitter*
KeyEventCollector::create_emitter()
{
  m_emitters.emplace_back(std::make_unique<KeyEventEmitter>(*this));
  return m_emitters.back().get();
}

void
KeyEventCollector::send(int value)
{
  assert(value == 0 || value == 1);

  if (value)
  {
    if (m_value >= static_cast<int>(m_emitters.size()))
    {
      log_error("got press event while all emitter where already pressed");
    }

    m_value += 1;

    if (m_value == 1)
    {
      m_uinput.send(get_device_id(), get_type(), get_code(), m_value);
    }
  }
  else
  {
    if (m_value <= 0)
    {
      log_error("got release event while collector was in release state");
    }

    m_value -= 1;

    if (m_value == 0)
    {
      m_uinput.send(get_device_id(), get_type(), get_code(), 0);
    }
  }
}

void
KeyEventCollector::sync()
{
}

} // namespace uinpp

/* EOF */
