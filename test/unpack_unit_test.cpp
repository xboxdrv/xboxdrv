/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011-2026 Ingo Ruhnke <grumbel@gmail.com>
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
*/

#include "unpack.hpp"
#include "unit_check.hpp"

using namespace xboxdrv;

int main()
{
  uint8_t data[] = { 0x04, 0x03, 0x02, 0x01 };

  CHECK_EQ(unpack::uint16be(data), static_cast<uint16_t>(0x0403));
  CHECK_EQ(unpack::uint16le(data), static_cast<uint16_t>(0x0304));
  CHECK_EQ(unpack::uint32be(data), static_cast<uint32_t>(0x04030201));
  CHECK_EQ(unpack::uint32le(data), static_cast<uint32_t>(0x01020304));

  // 0 -> -32768, 128 -> 0, 255 -> 32767
  CHECK_EQ(unpack::u8_to_s16(0), static_cast<int16_t>(-32768));
  CHECK_EQ(unpack::u8_to_s16(128), static_cast<int16_t>(0));
  CHECK_EQ(unpack::u8_to_s16(255), static_cast<int16_t>(32767));

  return test::exit_code();
}

/* EOF */
