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

#include "control.hpp"

#include "abs_port_in.hpp"
#include "abs_port_out.hpp"
#include "btn_port_in.hpp"
#include "btn_port_out.hpp"
#include "rel_port_in.hpp"
#include "rel_port_out.hpp"

Control::Control()
{
}

Control::~Control() 
{
  for(std::vector<BtnPortIn*>::iterator i = btn_port_in.begin(); i != btn_port_in.end(); ++i)
    delete *i;
  for(std::vector<BtnPortOut*>::iterator i = btn_port_out.begin(); i != btn_port_out.end(); ++i)
    delete *i;

  for(std::vector<AbsPortIn*>::iterator i = abs_port_in.begin(); i != abs_port_in.end(); ++i)
    delete *i;
  for(std::vector<AbsPortOut*>::iterator i = abs_port_out.begin(); i != abs_port_out.end(); ++i)
    delete *i;

  for(std::vector<RelPortIn*>::iterator i = rel_port_in.begin(); i != rel_port_in.end(); ++i)
    delete *i;
  for(std::vector<RelPortOut*>::iterator i = rel_port_out.begin(); i != rel_port_out.end(); ++i)
    delete *i;
}

BtnPortIn*
Control::get_btn_port_in(int idx)
{
  if (idx >= 0 && idx < (int)btn_port_in.size())
    return btn_port_in[idx];
  else
    return 0;
}

BtnPortOut*
Control::get_btn_port_out(int idx) 
{
  if (idx >= 0 && idx < (int)btn_port_out.size())
    return btn_port_out[idx]; 
  else
    return 0;
}

AbsPortIn*
Control::get_abs_port_in(int idx) 
{
  if (idx >= 0 && idx < (int)abs_port_in.size())
    return abs_port_in[idx];  
  else 
    {
      LOG("Couldn't find abs-in port " << idx);
      return 0;
    }
}

AbsPortOut*
Control::get_abs_port_out(int idx) 
{
  if (idx >= 0 && idx < (int)abs_port_out.size())
    return abs_port_out[idx]; 
  else
    {
      LOG("Couldn't find abs-out port " << idx);
      return 0;
    }
}

RelPortIn*
Control::get_rel_port_in(int idx) 
{
  if (idx >= 0 && idx < (int)rel_port_in.size())
    return rel_port_in[idx];  
  else 
    {
      LOG("Couldn't find rel-in port " << idx);
      return 0;
    }
}

RelPortOut*
Control::get_rel_port_out(int idx) 
{
  if (idx >= 0 && idx < (int)rel_port_out.size())
    return rel_port_out[idx]; 
  else
    {
      LOG("Couldn't find rel-out port " << idx);
      return 0;
    }
}

void connect_btn(Control* lhs_ctrl, int lhs_i, Control* rhs_ctrl, int rhs_i)
{
  BtnPortOut* out = lhs_ctrl->get_btn_port_out(lhs_i);
  BtnPortIn*  in  = rhs_ctrl->get_btn_port_in(rhs_i);

  if (in && out)
    out->connect(in);
  else
    LOG("Couldn't establish connection between " << lhs_ctrl << " and " << rhs_ctrl);
}

void connect_abs(Control* lhs_ctrl, int lhs_i, Control* rhs_ctrl, int rhs_i)
{
  AbsPortOut* out = lhs_ctrl->get_abs_port_out(lhs_i);
  AbsPortIn*  in  = rhs_ctrl->get_abs_port_in(rhs_i);

  if (in && out)
    out->connect(in);
  else
    LOG("Couldn't establish connection between " << lhs_ctrl << " and " << rhs_ctrl);
}

void connect_rel(Control* lhs_ctrl, int lhs_i, Control* rhs_ctrl, int rhs_i)
{
  RelPortOut* out = lhs_ctrl->get_rel_port_out(lhs_i);
  RelPortIn*  in  = rhs_ctrl->get_rel_port_in(rhs_i);

  if (in && out)
    out->connect(in);
  else
    LOG("Couldn't establish connection between " << lhs_ctrl << " and " << rhs_ctrl);
}

/* EOF */
