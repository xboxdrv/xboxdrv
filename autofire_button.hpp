/* 
**  Xbox/Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_AUTOFIRE_BUTTON_HPP
#define HEADER_AUTOFIRE_BUTTON_HPP

#include "control.hpp"

/** */
class AutofireButton : public Control
{
private:
  int rate;
  int rate_counter;
public:
  AutofireButton(int rate);
  virtual ~AutofireButton() {}
  std::string get_name() const { return "AutofireButton"; }

  void update(float delta);
  void on_btn(BtnPortOut* port);

private:
  AutofireButton (const AutofireButton&);
  AutofireButton& operator= (const AutofireButton&);
};

#endif

/* EOF */
