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

#include <iostream>

#include "unpack.hpp"

int main(int argc, char** argv)
{
  uint8_t data[] = { 0x04, 0x03, 0x02, 0x01 };

  std::cout << std::hex;
  std::cout << "uint16be: " << unpack::uint16be(data) << std::endl;
  std::cout << "uint16le: " << unpack::uint16le(data) << std::endl;

  std::cout << "uint32be: " << unpack::uint32be(data) << std::endl;
  std::cout << "uint32le: " << unpack::uint32le(data) << std::endl;

  return 0;
}

/* EOF */
