/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_XBOXDRV_LOG_HPP
#define HEADER_XBOXDRV_LOG_HPP

#include <iostream>

/** Takes __PRETTY_FUNCTION__ and tries to shorten it to the form:
    Classname::function() */
std::string log_pretty_print(const std::string& str);

/** informal status messages that don't indicate a fault in the
    program */
#define log_info  (std::cout << log_pretty_print(__PRETTY_FUNCTION__) << ": ")

/** messages that indicate an recoverable error (i.e. a catched
    exceptions) */
#define log_warning (std::cout << log_pretty_print(__PRETTY_FUNCTION__) << ": ")

/** things that shouldn't happen (i.e. a catched exceptions) */
#define log_error (std::cout << log_pretty_print(__PRETTY_FUNCTION__) << ": ")

/** extra verbose debugging messages */
#define log_debug (std::cout << log_pretty_print(__PRETTY_FUNCTION__) << ": ")

#endif

/* EOF */
