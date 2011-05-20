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

#ifndef HEADER_XBOXDRV_ENDIAN_HPP
#define HEADER_XBOXDRV_ENDIAN_HPP

#include <stdint.h>

namespace unpack {

inline bool is_big_endian()
{
  union {
    uint32_t i;
    char c[4];
  } bint = {0x01020304};

  return bint.c[0] == 1;
}

inline uint16_t swap16(uint16_t v)
{
  return (v<<8) | (v>>8);
}

inline uint32_t swap32(uint32_t v)
{
  return (v<<24) | ((v<<8) & 0x00ff0000) | ((v>>8) & 0x0000ff00) | (v>>24);
}


inline int16_t int16le(uint8_t* data)
{
  if (is_big_endian()) return swap16(*reinterpret_cast<int16_t*>(data));
  else                 return *reinterpret_cast<int16_t*>(data);
}

inline uint16_t uint16le(uint8_t* data)
{
  if (is_big_endian()) return swap16(*reinterpret_cast<uint16_t*>(data));
  else                 return *reinterpret_cast<uint16_t*>(data);
}


inline int32_t int32le(uint8_t* data)
{
  if (is_big_endian()) return swap32(*reinterpret_cast<int32_t*>(data));
  else                 return *reinterpret_cast<int32_t*>(data);
}

inline uint32_t uint32le(uint8_t* data)
{
  if (is_big_endian()) return swap32(*reinterpret_cast<uint32_t*>(data));
  else                 return *reinterpret_cast<uint32_t*>(data);
}



inline int16_t int16be(uint8_t* data)
{
  if (!is_big_endian()) return swap16(*reinterpret_cast<int16_t*>(data));
  else                  return *reinterpret_cast<int16_t*>(data);
}

inline uint16_t uint16be(uint8_t* data)
{
  if (!is_big_endian()) return swap16(*reinterpret_cast<uint16_t*>(data));
  else                  return *reinterpret_cast<uint16_t*>(data);
}


inline int32_t int32be(uint8_t* data)
{
  if (!is_big_endian()) return swap32(*reinterpret_cast<int32_t*>(data));
  else                  return *reinterpret_cast<int32_t*>(data);
}

inline uint32_t uint32be(uint8_t* data)
{
  if (!is_big_endian()) return swap32(*reinterpret_cast<uint32_t*>(data));
  else                  return *reinterpret_cast<uint32_t*>(data);
}


inline bool bit(uint8_t* data, int bit)
{
  return (*data >> bit) & 1;
}

} // namespace unpack

#endif

/* EOF */
