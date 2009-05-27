/* 
**  Xbox360 USB Gamepad Userspace Driver
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

#include "command_line_options.hpp"

CommandLineOptions::CommandLineOptions() 
{
  daemon   = false;
  verbose  = false;
  silent   = false;
  quiet    = false;
  rumble   = false;
  led      = -1;
  rumble_l = -1;
  rumble_r = -1;
  rumble_gain = 255;
  controller_id = 0;
  wireless_id   = 0;
  instant_exit = false;
  no_uinput = false;
  gamepad_type = GAMEPAD_UNKNOWN;
  busid[0] = '\0';
  devid[0] = '\0';
  vendor_id  = -1;
  product_id = -1;
  deadzone = 0;
  deadzone_trigger = 0;
  square_axis  = false;
}

CommandLineOptions* command_line_options = 0;

/* EOF */
