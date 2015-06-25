/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmail.com>
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

#include "bluetooth.hpp"

const bdaddr_t Bluetooth::addr_any   = {{0, 0, 0, 0, 0, 0}};
const bdaddr_t Bluetooth::addr_all   = {{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};
const bdaddr_t Bluetooth::addr_local = {{0, 0, 0, 0xff, 0xff, 0xff}};

/* EOF */
