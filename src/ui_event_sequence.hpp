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

#ifndef HEADER_XBOXDRV_UI_EVENT_SEQUENCE_HPP
#define HEADER_XBOXDRV_UI_EVENT_SEQUENCE_HPP

#include <vector>

#include "ui_event.hpp"

class UInput;

/** 
    A sequence of UIEvents (only key events allowed right now)
    
    FIXME: class name is kind of wrong
 */
class UIEventSequence
{
public:
  /** 
      "KEY_LEFTSHIFT+KEY_B" 
  */
  static UIEventSequence from_string(const std::string& value);

private:
  typedef std::vector<UIEvent> UIEvents;
  UIEvents m_sequence;

public:
  UIEventSequence();
  UIEventSequence(const UIEvents& sequence);
  UIEventSequence(const UIEvent& event);

  void init(UInput& uinput, int slot, bool extra_devices);
  void send(UInput& uinput, int value);

  void clear();

  std::string str() const;
};

#endif

/* EOF */
