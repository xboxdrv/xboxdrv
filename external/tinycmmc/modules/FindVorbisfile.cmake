# Copyright (C) 2021 Ingo Ruhnke <grumbel@gmail.com>
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

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
  pkg_search_module(PC_VORBISFILE vorbisfile)
endif()

find_path(VORBISFILE_INCLUDE_DIRECTORY vorbis/vorbisfile.h
  PATHS ${VORBISFILE_DIR} ${PC_VORBISFILE_INCLUDE_DIRS}
  PATH_SUFFIXES "include"
  )

find_library(VORBISFILE_LIBRARY vorbisfile
  PATHS ${VORBISFILE_DIR} ${PC_VORBISFILE_LIBRARY_DIRS}
  PATH_SUFFIXES "lib"
  )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Vorbisfile
  REQUIRED_VARS VORBISFILE_INCLUDE_DIRECTORY VORBISFILE_LIBRARY
  )

add_library(Vorbisfile::vorbisfile UNKNOWN IMPORTED)
set_target_properties(Vorbisfile::vorbisfile
  PROPERTIES
  IMPORTED_LOCATION "${VORBISFILE_LIBRARY}"
  INTERFACE_INCLUDE_DIRECTORIES "${VORBISFILE_INCLUDE_DIRECTORY};${PC_VORBISFILE_INCLUDE_DIRS}"
  INTERFACE_LINK_LIBRARIES "${PC_VORBISFILE_LINK_LIBRARIES}"
  INTERFACE_LINK_OPTIONS "${PC_VORBISFILE_LINK_OPTIONS}"
  INTERFACE_COMPILE_DEFINITIONS "${PC_VORBISFILE_COMPILE_DEFINITIONS}"
  INTERFACE_COMPILE_OPTIONS "${PC_VORBISFILE_COMPILE_OPTIONS}"
  )

# EOF #
