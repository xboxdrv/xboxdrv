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

#ifndef HEADER_UINPP_EVENT_COLLECTOR_HPP
#define HEADER_UINPP_EVENT_COLLECTOR_HPP

#include <cstdint>
#include <memory>
#include <vector>

#include "fwd.hpp"
#include "event_emitter.hpp"

namespace uinpp {

class EventCollector
{
protected:
  MultiDevice& m_uinput;
  uint32_t m_device_id;
  int m_type;
  int m_code;

public:
  EventCollector(MultiDevice& uinput, uint32_t device_id, int type, int code);
  virtual ~EventCollector();

  uint32_t get_device_id() const { return m_device_id; }
  int      get_type() const { return m_type; }
  int      get_code() const { return m_code; }

  virtual EventEmitter* create_emitter() = 0;
  virtual void sync() = 0;

private:
  EventCollector(EventCollector const&);
  EventCollector& operator=(EventCollector const&);
};

} // namespace uinpp

#endif

/* EOF */
