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

#include <boost/format.hpp>
#include <iostream>
#include "xboxmsg.hpp"

std::ostream& operator<<(std::ostream& out, const Xbox360GuitarMsg& msg) 
{
  out << boost::format(" whammy:%6d tilt:%6d | up:%d down:%d left:%d right:%d | back:%d guide:%d start:%d | green:%d red:%d yellow:%d blue:%d orange:%d ")
    % int(msg.whammy)
    % int(msg.tilt)
    % int(msg.dpad_up)
    % int(msg.dpad_down)
    % int(msg.dpad_left)
    % int(msg.dpad_right)
    % int(msg.back)
    % int(msg.guide)
    % int(msg.start)
    % int(msg.green)
    % int(msg.red)
    % int(msg.yellow)
    % int(msg.blue)
    % int(msg.orange);

  if (0)
    out << boost::format("| dummy: %d %d %d %d %02hhx %02hhx %04hx %04hx %02x %02x")
      % int(msg.thumb_l)
      % int(msg.thumb_r)
      % int(msg.rb)
      % int(msg.dummy1)

      % int(msg.lt)
      % int(msg.rt)

      % int16_t(msg.x1)
      % int16_t(msg.y1)

      % int(msg.dummy2)
      % int(msg.dummy3);
 
  return out;
}

std::ostream& operator<<(std::ostream& out, const Xbox360Msg& msg) 
{
  out << boost::format("S1:(%6d, %6d)") 
    % int(msg.x1) % int(msg.y1);

  out << boost::format("  S2:(%6d, %6d)")
    % int(msg.x2) % int(msg.y2);
                          
  out << boost::format(" [u:%d|d:%d|l:%d|r:%d]")
    % int(msg.dpad_up)
    % int(msg.dpad_down)
    % int(msg.dpad_left)
    % int(msg.dpad_right);

  out << "  back:" << msg.back;
  out << " guide:" << msg.guide;
  out << " start:" << msg.start;

  out << "  sl:" << msg.thumb_l;
  out << " sr:"  << msg.thumb_r;

  out << "  A:" << msg.a;
  out << " B:"  << msg.b;
  out << " X:"  << msg.x;
  out << " Y:"  << msg.y;

  out << "  LB:" << msg.lb;
  out << " RB:" <<  msg.rb;

  out << boost::format("  LT:%3d RT:%3d")
    % int(msg.lt) % int(msg.rt);

  if (0)
    out << " Dummy: " << msg.dummy1 << " " << msg.dummy2 << " " << msg.dummy3;

  return out;
}

std::ostream& operator<<(std::ostream& out, const XboxMsg& msg) 
{
  out << boost::format(" S1:(%6d, %6d) S2:(%6d, %6d) "
                       " [u:%d|d:%d|l:%d|r:%d] "
                       " start:%d back:%d "
                       " sl:%d sr:%d "
                       " A:%3d B:%3d X:%3d Y:%3d "
                       " black:%3d white:%3d "
                       " LT:%3d RT:%3d ")
    % int(msg.x1) % int(msg.y1)
    % int(msg.x2) % int(msg.y2)

    % int(msg.dpad_up)
    % int(msg.dpad_down)
    % int(msg.dpad_left)
    % int(msg.dpad_right)

    % int(msg.start)
    % int(msg.back)

    % int(msg.thumb_l)
    % int(msg.thumb_r)

    % int(msg.a)
    % int(msg.b)
    % int(msg.x)
    % int(msg.y)

    % int(msg.black)
    % int(msg.white)

    % int(msg.lt) 
    % int(msg.rt);

  return out;
}

/* EOF */
