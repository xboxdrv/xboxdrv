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

#include "command_line_options.hpp"

CommandLineOptions::CommandLineOptions() 
{
  daemon   = false;
  verbose  = false;
  silent   = false;
  rumble   = false;
  led      = -1;
  rumble_l = 0;
  rumble_r = 0;
  controller_id = 0;
  wireless_id   = 0;
  instant_exit = false;
  no_uinput = false;
  gamepad_type = GAMEPAD_UNKNOWN;
  busid[0] = '\0';
  devid[0] = '\0';
  deadzone = 0;
  square_axis  = false;
}

CommandLineOptions* command_line_options = 0;

/* EOF */
