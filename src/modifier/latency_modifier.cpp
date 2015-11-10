/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2012 Ingo Ruhnke <grumbel@gmail.com>
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

#include "modifier/latency_modifier.hpp"

#include <stdexcept>
#include <iostream>
#include <boost/lexical_cast.hpp>

#include "helper.hpp"
#include "raise_exception.hpp"

LatencyModifier*
LatencyModifier::from_string(const std::vector<std::string>& args)
{
  if (args.size() != 1)
  {
    raise_exception(std::runtime_error, "LatencyModifier requires one arguments");
  }
  else
  {
    return new LatencyModifier(str2int(args[0]));
  }
}

LatencyModifier::LatencyModifier(int latency) :
  m_latency(latency),
  m_time(0),
  m_messages(),
  m_times()
{
}

void
LatencyModifier::init(ControllerMessageDescriptor& desc)
{
}

void
LatencyModifier::update(int msec_delta, ControllerMessage& msg, const ControllerMessageDescriptor& desc)
{
  m_time += msec_delta;

  m_messages.push(msg);
  m_times.push(m_time + m_latency);

  msg = m_messages.front();
  while(m_times.front() < m_time)
  {
    m_times.pop();
    msg = m_messages.front();
    m_messages.pop();
  }
}


std::string
LatencyModifier::str() const
{
  return "latency";
}

/* EOF */
