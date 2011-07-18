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

#include <gtk/gtk.h>
#include <iostream>
#include <boost/bind.hpp>
#include <linux/input.h>

#include "log.hpp"
#include "ui_key_event_emitter.hpp"
#include "uinput.hpp"
#include "virtual_keyboard.hpp"

class KeyboardDispatcher
{
private:
  std::vector<UIEventEmitterPtr> m_emitter;

public:
  KeyboardDispatcher(VirtualKeyboard& gui_keyboard,
                     UInput& uinput) :
    m_emitter(KEY_CNT)
  {
    const KeyboardDescription& desc = gui_keyboard.get_description();

    for(int y = 0; y < desc.get_height(); ++y)
    {
      for(int x = 0; x < desc.get_width(); ++x)
      {
        const Key& key = desc.get_key(x, y);
        if (key.m_code != -1)
        {
          m_emitter[key.m_code] = uinput.add_key(0, key.m_code);
        }
      }
    }

    gui_keyboard.set_key_callback(boost::bind(&KeyboardDispatcher::on_key, this, _1, _2));
  }

  void on_key(const Key& key, bool pressed)
  {
    log_tmp(key.m_code << " " << pressed);
    /*
    if (key.m_code != -1)
    {
      m_emitter[key.m_code]->send(pressed);
    }*/
  }
};

int main(int argc, char** argv)
{
  gtk_init(&argc, &argv);

  UInput uinput(false);

  KeyboardDescription keyboard_desc(KeyboardDescription::create_us_layout()); 
  VirtualKeyboard virtual_keyboard(keyboard_desc);
  KeyboardDispatcher dispatcher(virtual_keyboard, uinput);

  uinput.finish();

  virtual_keyboard.show();

  gtk_main();

  return 0;
}

/* EOF */
