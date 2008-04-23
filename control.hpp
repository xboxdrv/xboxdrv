
**   __      __ __             ___        __   __ __   __
**  /  \    /  \__| ____    __| _/_______/  |_|__|  | |  |   ____
**  \   \/\/   /  |/    \  / __ |/  ___/\   __\  |  | |  | _/ __ \
**   \        /|  |   |  \/ /_/ |\___ \  |  | |  |  |_|  |_\  ___/
**    \__/\  / |__|___|  /\____ /____  > |__| |__|____/____/\___  >
**         \/          \/      \/    \/                         \/
**  Copyright (C) 2007 Ingo Ruhnke <grumbel@gmx.de>
**
**  This program is free software; you can redistribute it and/or
**  modify it under the terms of the GNU General Public License
**  as published by the Free Software Foundation; either version 2
**  of the License, or (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
** 
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
**  02111-1307, USA.
*/

#ifndef HEADER_CONTROL_HPP
#define HEADER_CONTROL_HPP

#include <boost/signal.hpp>


class Button
{
protected:
  bool down;

public:
  Button()
    : down(true)
  {}

  bool get_state() const { return down; }

  void set_state(bool new_state)
  {
    if (new_state != down) 
      {
        down = new_state;
        sig_on_change(this);
      }
  }

  boost::signal<Button*> sig_on_change;
};

class UInputButton : public Button 
{
protected:
  UInput*  uinput;
  uint16_t code;

public:
  UInputButton(UInput* uinput, uint16_t code) 
  : uinput(uinput), 
    code(code)
  {}  

  void on_child_change(Button* button)
  {
    state = button->get_state();
    uinput->send_button(code, state);
  }
};

#endif

/* EOF */
