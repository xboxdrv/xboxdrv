/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2026 Ingo Ruhnke <grumbel@gmail.com>
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
*/

#ifndef HEADER_XBOXDRV_XBOXDRV_VERSION_HPP
#define HEADER_XBOXDRV_XBOXDRV_VERSION_HPP

namespace xboxdrv {

/** Full package version string (from VERSION + optional git rev). */
char const* xboxdrv_version();

} // namespace xboxdrv

#endif

/* EOF */
