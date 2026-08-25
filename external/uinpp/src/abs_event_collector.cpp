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

#include "abs_event_collector.hpp"

#include "multi_device.hpp"

namespace uinpp {

AbsEventCollector::AbsEventCollector(MultiDevice& uinput, uint32_t device_id, int type, int code) :
  EventCollector(uinput, device_id, type, code),
  m_emitters()
{
}

EventEmitter*
AbsEventCollector::create_emitter()
{
  m_emitters.emplace_back(std::make_unique<AbsEventEmitter>(*this));
  return m_emitters.back().get();
}

void
AbsEventCollector::send(int value)
{
  m_uinput.send(get_device_id(), get_type(), get_code(), value);
}

void
AbsEventCollector::sync()
{
}

} // namespace uinpp

/* EOF */
