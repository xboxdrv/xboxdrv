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

#ifndef HEADER_XBOXDRV_ENDIAN_HPP
#define HEADER_XBOXDRV_ENDIAN_HPP

#include <stdint.h>

#include "util/math.hpp"

namespace xboxdrv {

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
  return static_cast<uint16_t>((v<<8) | (v>>8));
}

inline uint32_t swap32(uint32_t v)
{
  return (v<<24) | ((v<<8) & 0x00ff0000) | ((v>>8) & 0x0000ff00) | (v>>24);
}


inline int16_t int16le(const uint8_t* data)
{
  if (is_big_endian()) return swap16(*reinterpret_cast<const int16_t*>(data));
  else                 return *reinterpret_cast<const int16_t*>(data);
}

inline uint16_t uint16le(const uint8_t* data)
{
  if (is_big_endian()) return swap16(*reinterpret_cast<const uint16_t*>(data));
  else                 return *reinterpret_cast<const uint16_t*>(data);
}


inline int32_t int32le(const uint8_t* data)
{
  if (is_big_endian()) return swap32(*reinterpret_cast<const int32_t*>(data));
  else                 return *reinterpret_cast<const int32_t*>(data);
}

inline uint32_t uint32le(const uint8_t* data)
{
  if (is_big_endian()) return swap32(*reinterpret_cast<const uint32_t*>(data));
  else                 return *reinterpret_cast<const uint32_t*>(data);
}



inline int16_t int16be(const uint8_t* data)
{
  if (!is_big_endian()) return swap16(*reinterpret_cast<const int16_t*>(data));
  else                  return *reinterpret_cast<const int16_t*>(data);
}

inline uint16_t uint16be(const uint8_t* data)
{
  if (!is_big_endian()) return swap16(*reinterpret_cast<const uint16_t*>(data));
  else                  return *reinterpret_cast<const uint16_t*>(data);
}


inline int32_t int32be(const uint8_t* data)
{
  if (!is_big_endian()) return swap32(*reinterpret_cast<const int32_t*>(data));
  else                  return *reinterpret_cast<const int32_t*>(data);
}

inline uint32_t uint32be(const uint8_t* data)
{
  if (!is_big_endian()) return swap32(*reinterpret_cast<const uint32_t*>(data));
  else                  return *reinterpret_cast<const uint32_t*>(data);
}


inline bool bit(const uint8_t* data, int bit)
{
  return (*data >> bit) & 1;
}

// Change the sign
inline int16_t s16_invert(int16_t v)
{
  if (v)
    return static_cast<int16_t>(~v);
  else // v == 0
    return v;
}

inline int16_t s8_to_s16(int8_t v)
{
  if (v > 0)
    return static_cast<int16_t>(v * 32767 / 127);
  else
    return static_cast<int16_t>(v * 32768 / 128);
}

inline int16_t u8_to_s16(uint8_t value)
{
  int16_t v = value;
  if (v > 128)
  {
    return static_cast<int16_t>((v-128) * 32767 / 127);
  }
  else
  {
    return static_cast<int16_t>((v-128) * 32768 / 128);
  }
}

inline float s16_to_float(int16_t value)
{
  if (value >= 0)
  {
    return static_cast<float>(value) / 32767.0f;
  }
  else
  {
    return static_cast<float>(value) / 32768.0f;
  }
}

/**
   input:  [0, 255]
   output: [ -1.0f, 1.0f ]
*/
inline float u8_to_float(uint8_t value)
{
  return static_cast<float>(value) / 255.0f * 2.0f - 1.0f;
}

inline int16_t float_to_s16(float v)
{
  if (v >= 0.0f)
  {
    return static_cast<int16_t>(std::min(1.0f, v) * 32767.0f);
  }
  else
  {
    return static_cast<int16_t>(std::max(-1.0f, v) * 32768.0f);
  }
}

/**
   input:  [ -1.0f, 1.0f ]
   output: [0, 255]
*/
inline uint8_t float_to_u8(float v)
{
  return static_cast<uint8_t>(std::clamp((v + 1.0f) / 2.0f, 0.0f, 1.0f) * 255.0f);
}

} // namespace unpack

} // namespace xboxdrv

#endif

/* EOF */
