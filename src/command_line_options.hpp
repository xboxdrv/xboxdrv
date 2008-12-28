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

#ifndef HEADER_COMMAND_LINE_OPTIONS_HPP
#define HEADER_COMMAND_LINE_OPTIONS_HPP

#include <vector>
#include "modifier.hpp"
#include "xboxmsg.hpp"
#include "uinput.hpp"

struct CommandLineOptions 
{
  bool daemon;
  bool verbose;
  bool silent;
  bool rumble;
  int  led;
  int  rumble_l;
  int  rumble_r;
  int  controller_id;
  int  wireless_id;
  bool instant_exit;
  bool no_uinput;
  GamepadType gamepad_type;
  char busid[4];
  char devid[4];
  uInputCfg uinput_config;
  int deadzone;
  std::vector<ButtonMapping> button_map;
  std::vector<AxisMapping>   axis_map;
  std::vector<AutoFireMapping> autofire_map;
  std::vector<RelativeAxisMapping> relative_axis_map;
  bool square_axis;

  CommandLineOptions();
};

extern CommandLineOptions* command_line_options;

#endif

/* EOF */
