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

#include "event_sequence.hpp"

#include <sstream>

#include <strut/split.hpp>

#include "event.hpp"
#include "multi_device.hpp"

namespace uinpp {

EventSequence::EventSequence() :
  m_sequence(),
  m_emitters()
{
}

EventSequence::EventSequence(std::vector<Event> const& sequence) :
  m_sequence(sequence),
  m_emitters()
{
}

EventSequence::EventSequence(Event const& event) :
  m_sequence(1, event),
  m_emitters()
{
}

void
EventSequence::init(MultiDevice& uinput, int slot, bool extra_devices)
{
  for(auto i = m_sequence.begin(); i != m_sequence.end(); ++i)
  {
    i->resolve_device_id(slot, extra_devices);
    m_emitters.push_back(uinput.add_key(i->get_device_id(), i->code));
  }
}

void
EventSequence::send(int value)
{
  if (value)
  {
    for(auto i = m_emitters.begin(); i != m_emitters.end(); ++i)
    {
      (*i)->send(value);
    }
  }
  else
  {
    // on release, send events in reverse order
    for(auto i = m_emitters.rbegin(); i != m_emitters.rend(); ++i)
    {
      (*i)->send(value);
    }
  }
}

void
EventSequence::clear()
{
  m_sequence.clear();
}

std::string
EventSequence::str() const
{
  std::ostringstream out;

  for(auto i = m_sequence.begin(); i != m_sequence.end(); ++i)
  {
    out << i->get_device_id() << "-" << i->code;

    if (i != m_sequence.end()-1)
      out << "+";
  }

  return out.str();
}

} // namespace uinpp

/* EOF */
