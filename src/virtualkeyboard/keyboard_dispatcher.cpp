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

#include "virtualkeyboard/keyboard_dispatcher.hpp"

#include <linux/input.h>
#include <functional>

#include <uinpp/uinput.hpp>

#include "log.hpp"
#include "virtualkeyboard/keyboard_description.hpp"
#include "virtualkeyboard/virtual_keyboard.hpp"

using namespace std::placeholders;

KeyboardDispatcher::KeyboardDispatcher(VirtualKeyboard& gui_keyboard,
                                       UInput& uinput) :
  m_uinput(uinput),
  m_emitter(KEY_CNT)
{
  const KeyboardDescription& desc = *gui_keyboard.get_description();

  for(int y = 0; y < desc.get_height(); ++y)
  {
    for(int x = 0; x < desc.get_width(); ++x)
    {
      Key* key = desc.get_key(x, y);
      if (key)
      {
        m_emitter[key->get_code()] = uinput.add_key(0, key->get_code());
      }
    }
  }

  gui_keyboard.set_key_callback(std::bind(&KeyboardDispatcher::on_key, this, _1, _2));
}

void
KeyboardDispatcher::on_key(const Key& key, bool pressed)
{
  log_tmp("emitting: " << key.get_code() << " " << pressed);
  m_emitter[key.get_code()]->send(pressed);
  m_uinput.sync();
}

/* EOF */
