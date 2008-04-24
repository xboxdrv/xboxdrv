/*
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


struct ButtonPortIn
{
  std::string label;
  boost::function<void(ButtonPortOut*)> on_change;
  ButtonPortOut* out_port;

  ButtonPortIn(const std::string& label, const boost::function<void(void)>& on_change)
    : label(label), on_change(on_change), out_port(0) {}
};

class ButtonPortOut
{
public:
  std::string label;

  boost::signal<void(ButtonPortOut*)> sig_change;

  // true if pressed, false otherwise
  bool state;

  ButtonPortOut(const std::string& label) 
    : label(label) {}


  std::string get_label() { return label; }
  bool get_state() { return state; }
};

class Control
{
protected:
  std::vector<ButtonPortIn*>  btn_port_in;
  std::vector<ButtonPortOut*> btn_port_out;

public:
  Control() {
  }

  ~Control() 
  {
    for(std::vector<ButtonPortIn*>::iterator i = btn_port_in.begin(); i != btn_port_in.end(); ++i)
      delete *i;

    for(std::vector<ButtonPortOut*>::iterator i = btn_port_out.begin(); i != btn_port_out.end(); ++i)
      delete *i;
  }

  int get_btn_port_in_count()  { return btn_port_in;  }
  int get_btn_port_out_count() { return btn_port_out; }

  BtnPortIn*  get_btn_port_in(int idx)  { return btn_port_in[idx];  }
  BtnPortOut* get_btn_port_out(int idx) { return btn_port_out[idx]; }
};

class UInputButton : public Control 
{
protected:
  UInput*  uinput;
  uint16_t code;

public:
  UInputButton(UInput* uinput, uint16_t code) 
  : uinput(uinput), 
    code(code)
  {
    btn_port_in.push_back(new BtnPortIn((boost::format("UInput-Button %hd") % code).str(), 
                                        boost::bind(this, &UInputButton::on_button)));
  }

  void on_button(ButtonPortOut* btn)
  {
    uinput->send_button(code, btn->get_state());
  }
};

#endif

/* EOF */
