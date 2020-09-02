/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_XBOXDRV_BUTTONEVENT_KEY_BUTTON_EVENT_HANDLER_HPP
#define HEADER_XBOXDRV_BUTTONEVENT_KEY_BUTTON_EVENT_HANDLER_HPP

#include "button_event.hpp"

#include "uinput/ui_event_sequence.hpp"

class KeyButtonEventHandler : public ButtonEventHandler
{
public:
  static KeyButtonEventHandler* from_string(UInput& uinput, int slot, bool extra_devices,
                                            const std::string& str);

public:
  KeyButtonEventHandler(UInput& uinput, int slot, bool extra_devices,
                        const UIEventSequence& codes,
                        const UIEventSequence& secondary_codes,
                        int m_hold_threshold);

  void send(bool value);
  void update(int msec_delta);

  std::string str() const;

private:
  bool m_state;
  UIEventSequence m_codes;
  UIEventSequence m_secondary_codes;

  int m_hold_threshold;
  int m_hold_counter;
  int m_release_scheduled;
};

#endif

/* EOF */
