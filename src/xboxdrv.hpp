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

#ifndef HEADER_XBOX360_HPP
#define HEADER_XBOX360_HPP

#include "xboxmsg.hpp"

class CommandLineOptions;
class uInput;
class XboxGenericController;

class Xboxdrv
{
private:
  void print_command_line_help(int argc, char** argv);
  void print_led_help();
  void print_version();
  void run_main(const CommandLineOptions& opts);
  void parse_command_line(int argc, char** argv, CommandLineOptions& opts);
  void controller_loop(GamepadType type, uInput* uinput,
                       XboxGenericController* controller, 
                       const CommandLineOptions& opts);

public:
  int main(int argc, char** argv);
};

#endif

/* EOF */
