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

#ifndef HEADER_XBOXDRV_VIRTUALKEYBOARD_KEYBOARD_DISPATCHER_HPP
#define HEADER_XBOXDRV_VIRTUALKEYBOARD_KEYBOARD_DISPATCHER_HPP

#include <vector>

#include "ui_event_emitter.hpp"

class Key;
class UInput;
class VirtualKeyboard;

/** The KeyboardDispatcher receives key events from the
    VirtualKeyboard and passes them on to UInput to translate them
    into device driver level events. It also takes care of creates
    the UInput devices in the first place. */
class KeyboardDispatcher
{
private:
  UInput& m_uinput;
  std::vector<UIEventEmitterPtr> m_emitter;

public:
  KeyboardDispatcher(VirtualKeyboard& gui_keyboard, UInput& uinput);
  void on_key(const Key& key, bool pressed);
};

#endif

/* EOF */
