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

class SixAxisMsg
{
public:
  unsigned int dummy1 :8; // always 1
  // --------------------------
  unsigned int dummy2 :8; // always 0
  // --------------------------
  unsigned int select     :1;
  unsigned int l3         :1;
  unsigned int r3         :1;
  unsigned int start      :1;

  unsigned int dpad_up    :1;
  unsigned int dpad_right :1;
  unsigned int dpad_down  :1;
  unsigned int dpad_left  :1;
  // --------------------------
  unsigned int l2         :1;
  unsigned int r2         :1;
  unsigned int l1         :1;
  unsigned int r1         :1;
  
  unsigned int triangle   :1;
  unsigned int circle     :1;
  unsigned int cross      :1;
  unsigned int square     :1;
  // --------------------------
  unsigned int ps_btn     :1;
  unsigned int dummy3     :7; // always 0
  // --------------------------
  unsigned int dummy4     :8; // always 0
  // Analog stuff follows -----
  unsigned int x1         :8; 
  unsigned int y1         :8; 
  unsigned int x2         :8; 
  unsigned int y2         :8; 
  // --------------------------
  unsigned int dummy5     :32; // always 0
  // --------------------------
  unsigned int dpad_up;    :8;
  unsigned int dpad_right; :8;
  unsigned int dpad_down;  :8;
  unsigned int dpad_left;  :8;
  // --------------------------
  unsigned int l2          :8;
  unsigned int r2          :8;
  unsigned int l1          :8;
  unsigned int r1          :8;
  // --------------------------
  unsigned int triangle    :8;
  unsigned int circle      :8;
  unsigned int cross       :8;
  unsigned int square      :8;
  // --------------------------
  unsigned int dummy6      :120; // always: 0x00, 0x00, 0x00, 0x06, 0xee, 0x10, 0x00, 0x00, 0x00, 0x00, 0x06, 0x83, 0x77, 0x01, 0x81
  // --------------------------
  unsigned int xacl_sign   :8; // 0x02 or 0x01
  unsigned int xacl_value  :8;

  unsigned int y_acl_sign   :8; // 0x02 or 0x01
  unsigned int y_acl_value  :8 

  unsigned int z_acl_sign   :8; // 0x02 or 0x01
  unsigned int z_acl_value  :8 

  unsigned int unknown      :8;
  unsigned int unknown_sign :8;
};

/* EOF */
