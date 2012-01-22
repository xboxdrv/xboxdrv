/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2012 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_XBOXDRV_MODIFIER_LATENCY_MODIFIER_HPP
#define HEADER_XBOXDRV_MODIFIER_LATENCY_MODIFIER_HPP

#include <queue>

#include "modifier.hpp"

class LatencyModifier : public Modifier
{
public:
  static LatencyModifier* from_string(const std::vector<std::string>& args);

private:
  int m_latency;
  int m_time;
  std::queue<ControllerMessage> m_messages;
  std::queue<int> m_times;

public:
  LatencyModifier(int latency);
  ~LatencyModifier() {}

  void init(ControllerMessageDescriptor& desc);
  void update(int msec_delta, ControllerMessage& msg, const ControllerMessageDescriptor& desc);

  std::string str() const;

private:
  LatencyModifier(const LatencyModifier&);
  LatencyModifier& operator=(const LatencyModifier&);
};

#endif

/* EOF */
