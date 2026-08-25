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

#ifndef HEADER_UINPP_EVENT_HPP
#define HEADER_UINPP_EVENT_HPP

#include <cstdint>
#include <string>

namespace uinpp {

enum {
  DEVICEID_INVALID  = static_cast<uint16_t>(-5),
  DEVICEID_KEYBOARD = static_cast<uint16_t>(-4),
  DEVICEID_MOUSE    = static_cast<uint16_t>(-3),
  DEVICEID_JOYSTICK = static_cast<uint16_t>(-2),
  DEVICEID_AUTO     = static_cast<uint16_t>(-1)
};

enum {
  SLOTID_AUTO = static_cast<uint16_t>(-1)
};

class Event
{
public:
  static Event create(uint16_t device_id, int type, int code);
  static Event invalid();

public:
  void resolve_device_id(int slot, bool extra_devices);
  bool operator<(Event const& rhs)  const;

  int type;
  int code;

  int get_type() const { return type; }
  int get_code() const { return code; }
  uint32_t get_device_id() const;

public: // FIXME: temporarily made public for compatiblity
  uint16_t m_slot_id;
  uint16_t m_device_id;
  bool m_device_id_resolved;
};

} // namespace uinpp

#endif

/* EOF */
