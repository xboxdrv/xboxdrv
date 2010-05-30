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

#ifndef HEADER_CONTROL_HPP
#define HEADER_CONTROL_HPP

#include <boost/signal.hpp>
#include "log.hpp"

class BtnPortIn;
class BtnPortOut;

class AbsPortIn;
class AbsPortOut;

class RelPortIn;
class RelPortOut;

class Control
{
protected:
  std::vector<BtnPortIn*>  btn_port_in;
  std::vector<BtnPortOut*> btn_port_out;

  std::vector<AbsPortIn*>  abs_port_in;
  std::vector<AbsPortOut*> abs_port_out;

  std::vector<RelPortIn*>  rel_port_in;
  std::vector<RelPortOut*> rel_port_out;

public:
  Control();
  virtual ~Control();

  virtual std::string get_name() const =0;

  int get_btn_port_in_count()  { return btn_port_in.size();  }
  int get_btn_port_out_count() { return btn_port_out.size(); }

  BtnPortIn*  get_btn_port_in(int idx);
  BtnPortOut* get_btn_port_out(int idx);


  int get_abs_port_in_count()  { return abs_port_in.size();  }
  int get_abs_port_out_count() { return abs_port_out.size(); }

  AbsPortIn*  get_abs_port_in(int idx);
  AbsPortOut* get_abs_port_out(int idx);


  int get_rel_port_in_count()  { return rel_port_in.size();  }
  int get_rel_port_out_count() { return rel_port_out.size(); }

  RelPortIn*  get_rel_port_in(int idx);
  RelPortOut* get_rel_port_out(int idx);


  virtual void update(float delta) {}
};

void connect_btn(Control* lhs_ctrl, int lhs_i, Control* rhs, int rhs_i);
void connect_abs(Control* lhs_ctrl, int lhs_i, Control* rhs, int rhs_i);
void connect_rel(Control* lhs_ctrl, int lhs_i, Control* rhs, int rhs_i);

#endif

/* EOF */
