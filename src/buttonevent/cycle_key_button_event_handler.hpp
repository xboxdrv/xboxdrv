/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_XBOXDRV_BUTTONEVENT_CYCLE_KEY_BUTTON_EVENT_HANDLER_HPP
#define HEADER_XBOXDRV_BUTTONEVENT_CYCLE_KEY_BUTTON_EVENT_HANDLER_HPP

#include <map>

#include "button_event.hpp"
#include "ui_event_sequence.hpp"

class CycleKeyButtonEventHandler : public ButtonEventHandler
{
private:
  static std::map<std::string, CycleKeyButtonEventHandler*> s_lookup_table;

public:
  static CycleKeyButtonEventHandler* from_string(const std::string& str);
  static CycleKeyButtonEventHandler* from_string_named(const std::string& str);
  static CycleKeyButtonEventHandler* lookup(const std::string& name);

private:
  typedef std::vector<UIEventSequence> Keys;
  Keys m_keys;
  int m_current_key;

private:
  CycleKeyButtonEventHandler(const Keys& keys);

public:
  void init(UInput& uinput, int slot, bool extra_devices);
  void send(UInput& uinput, bool value);
  void send_only(UInput& uinput, bool value);
  void update(UInput& uinput, int msec_delta);

  void next_key();
  void prev_key();

  std::string str() const;

private:
  CycleKeyButtonEventHandler(const CycleKeyButtonEventHandler&);
  CycleKeyButtonEventHandler& operator=(const CycleKeyButtonEventHandler&);
};

#endif

/* EOF */
