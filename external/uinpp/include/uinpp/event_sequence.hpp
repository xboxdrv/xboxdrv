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

#ifndef HEADER_NPP_EVENT_SEQUENCE_HPP
#define HEADER_NPP_EVENT_SEQUENCE_HPP

#include <vector>

#include "event.hpp"
#include "event_emitter.hpp"

namespace uinpp {

/**
    A sequence of Events (only key events allowed right now)

    FIXME: class name is kind of wrong
 */
class EventSequence
{
public:
  EventSequence();
  EventSequence(std::vector<Event> const& sequence);
  EventSequence(Event const& event);

  void init(MultiDevice& uinput, int slot, bool extra_devices);
  void send(int value);

  void clear();

  std::string str() const;

private:
  std::vector<Event> m_sequence;
  std::vector<EventEmitter*> m_emitters;
};

} // namespace uinpp

#endif

/* EOF */
