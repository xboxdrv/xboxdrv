/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2026 Ingo Ruhnke <grumbel@gmail.com>
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
*/

#include "xboxdrv_version.hpp"

// PACKAGE_VERSION comes from the generated xboxdrv_version_def.hpp
// (build dir). Only this TU includes it so version bumps rebuild one object.
#include "xboxdrv_version_def.hpp"
#ifndef PACKAGE_VERSION
#  define PACKAGE_VERSION "unknown"
#endif

namespace xboxdrv {

char const* xboxdrv_version()
{
  return PACKAGE_VERSION;
}

} // namespace xboxdrv

/* EOF */
