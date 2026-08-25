# Copyright (C) 2022 Ingo Ruhnke <grumbel@gmail.com>
#
# This software is provided 'as-is', without any express or implied
# warranty.  In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.

find_path(SPNAV_INCLUDE_DIRECTORY spnav.h
  PATHS ${SPNAV_DIR}
  PATH_SUFFIXES "include"
  )

find_library(SPNAV_LIBRARY spnav
  PATHS ${SPNAV_DIR}
  PATH_SUFFIXES "lib"
  )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(spnav
  REQUIRED_VARS SPNAV_LIBRARY SPNAV_INCLUDE_DIRECTORY
  )

add_library(spnav::spnav UNKNOWN IMPORTED)
set_target_properties(spnav::spnav
  PROPERTIES
  IMPORTED_LOCATION "${SPNAV_LIBRARY}"
  INTERFACE_INCLUDE_DIRECTORIES "${SPNAV_INCLUDE_DIRECTORY}"
  )

# EOF #
