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

// PACKAGE_VERSION is supplied only for this translation unit via
// set_source_files_properties in CMakeLists.txt so a version/git-rev
// change does not force a full libxboxdrv rebuild.
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
