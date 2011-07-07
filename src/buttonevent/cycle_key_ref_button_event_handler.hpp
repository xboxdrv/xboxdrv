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

#ifndef HEADER_XBOXDRV_BUTTONEVENT_CYCLE_KEY_REF_BUTTON_EVENT_HANDLER_HPP
#define HEADER_XBOXDRV_BUTTONEVENT_CYCLE_KEY_REF_BUTTON_EVENT_HANDLER_HPP

#include "button_event.hpp"

class CycleKeyButtonEventHandler;

class CycleKeyRefButtonEventHandler : public ButtonEventHandler
{
public:
  /** 
      Syntax: "{direction}:{press}"
      
      direction: can either be 'forward', 'backward', 'none' or an
      integer, in the case of an integer, the pointer is moved to that key
      
      press: a bool, true if a keypress is send, 
      false when only the current key should change
  */
  static CycleKeyRefButtonEventHandler* from_string(const std::string& value);

public:
  enum Direction { kForward, kBackward, kNone };

public:
  CycleKeyRefButtonEventHandler(CycleKeyButtonEventHandler* button_handler, 
                                Direction direction, 
                                bool press);

  void init(UInput& uinput, int slot, bool extra_devices);
  void send(UInput& uinput, bool value);
  void update(UInput& uinput, int msec_delta);

  std::string str() const;

private:
  CycleKeyButtonEventHandler* m_button_handler;
  Direction m_direction;
  bool m_send_press;

private:
  CycleKeyRefButtonEventHandler(const CycleKeyRefButtonEventHandler&);
  CycleKeyRefButtonEventHandler& operator=(const CycleKeyRefButtonEventHandler&);
};

#endif

/* EOF */
