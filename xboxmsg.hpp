/*  $Id$
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

#ifndef HEADER_XBOXMSG_HPP
#define HEADER_XBOXMSG_HPP

#include <iosfwd>

struct Xbox360Msg
{
  // -------------------------
  unsigned int type       :8;
  unsigned int length     :8;

  // data[2] ------------------
  unsigned int dpad_up     :1;
  unsigned int dpad_down   :1;
  unsigned int dpad_left   :1;
  unsigned int dpad_right  :1;

  unsigned int start       :1;
  unsigned int back        :1;

  unsigned int thumb_l     :1;
  unsigned int thumb_r     :1;

  // data[3] ------------------
  unsigned int lb          :1;
  unsigned int rb          :1;
  unsigned int guide       :1;
  unsigned int dummy1      :1;

  unsigned int a           :1;
  unsigned int b           :1;
  unsigned int x           :1;
  unsigned int y           :1;

  // data[4] ------------------
  unsigned int lt          :8;
  unsigned int rt          :8;

  // data[6] ------------------
  int x1                   :16;
  int y1                   :16;

  // data[10] -----------------
  int x2                   :16;
  int y2                   :16;

  // data[14]; ----------------
  unsigned int dummy2      :32;
  unsigned int dummy3      :16;
} __attribute__((__packed__));

struct Xbox360GuitarMsg
{
  // -------------------------
  unsigned int type       :8;
  unsigned int length     :8;

  // data[2] ------------------
  unsigned int dpad_up     :1; // also strum-up
  unsigned int dpad_down   :1; // also strum-down
  unsigned int dpad_left   :1;
  unsigned int dpad_right  :1;

  unsigned int start       :1;
  unsigned int back        :1;

  unsigned int thumb_l     :1; // unused
  unsigned int thumb_r     :1; // unused

  // data[3] ------------------
  unsigned int orange      :1; // 5
  unsigned int rb          :1; // unused
  unsigned int guide       :1; 
  unsigned int dummy1      :1; // unused

  unsigned int green       :1; // 1, A 
  unsigned int red         :1; // 2, B
  unsigned int blue        :1; // 4, X
  unsigned int yellow      :1; // 3, Y

  // data[4] ------------------
  unsigned int lt          :8; // unknown
  unsigned int rt          :8; // unknown

  // data[6] ------------------
  int x1                   :16; // unused
  int y1                   :16; // unused

  // data[10] -----------------
  int whammy               :16;
  int tilt                 :16;

  // data[14]; ----------------
  unsigned int dummy2      :32; // unused
  unsigned int dummy3      :16; // unused
} __attribute__((__packed__));

struct XboxMsg
{
  // --------------------------
  unsigned int type       :8;
  unsigned int length     :8;

  // data[2] ------------------
  unsigned int dpad_up     :1;
  unsigned int dpad_down   :1;
  unsigned int dpad_left   :1;
  unsigned int dpad_right  :1;

  unsigned int start       :1;
  unsigned int back        :1;

  unsigned int thumb_l     :1;
  unsigned int thumb_r     :1;

  // data[3] ------------------
  unsigned int dummy       :8;
  unsigned int a           :8;
  unsigned int b           :8;
  unsigned int x           :8;
  unsigned int y           :8;
  unsigned int black       :8;
  unsigned int white       :8;
  unsigned int lt          :8;
  unsigned int rt          :8;

  // data[6] ------------------
  int x1                   :16;
  int y1                   :16;

  // data[10] -----------------
  int x2                   :16;
  int y2                   :16;
} __attribute__((__packed__));

std::ostream& operator<<(std::ostream& out, const Xbox360GuitarMsg& msg);
std::ostream& operator<<(std::ostream& out, const Xbox360Msg& msg);
std::ostream& operator<<(std::ostream& out, const XboxMsg& msg);

#endif

/* EOF */
