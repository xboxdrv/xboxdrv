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

#ifndef HEADER_UINPP_KEY_EVENT_EMITTER_HPP
#define HEADER_UINPP_KEY_EVENT_EMITTER_HPP

#include "event_emitter.hpp"

namespace uinpp {

class KeyEventCollector;

class KeyEventEmitter : public EventEmitter
{
public:
  KeyEventEmitter(KeyEventCollector& collector);

  void send(int value) override;

private:
  KeyEventCollector& m_collector;
  bool m_value;

private:
  KeyEventEmitter(KeyEventEmitter const&);
  KeyEventEmitter& operator=(KeyEventEmitter const&);
};

} // namespace uinpp

#endif

/* EOF */
