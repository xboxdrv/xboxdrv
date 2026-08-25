# Copyright (C) 2020 Ingo Ruhnke <grumbel@gmail.com>
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

pkg_search_module(SQUIRREL squirrel3 IMPORTED_TARGET)
if(NOT SQUIRREL_FOUND)
  find_library(SQUIRREL_LIBRARIES NAMES "squirrel" "squirrel3" REQUIRED)
  find_library(SQSTDLIB_LIBRARIES NAMES "sqstdlib" "sqstdlib3" REQUIRED)
  find_path(SQUIRREL_INCLUDE_DIRS NAMES "squirrel.h" PATH_SUFFIXES "/" "squirrel/" "squirrel3/")
  if (NOT SQUIRREL_INCLUDE_DIRS)
    message(FATAL_ERROR "squirrel not found")
  endif()

  # merge sqstdlib and squirrel, since pkg-config doesn't tell them appart
  set(SQUIRREL_LIBRARIES ${SQUIRREL_LIBRARIES} ${SQSTDLIB_LIBRARIES})

  add_library(PkgConfig::SQUIRREL INTERFACE IMPORTED)
  target_link_libraries(PkgConfig::SQUIRREL INTERFACE ${SQUIRREL_LIBRARIES})
  target_include_directories(PkgConfig::SQUIRREL INTERFACE ${SQUIRREL_INCLUDE_DIRS})
  message(STATUS "Found Squirrel: ${SQUIRREL_LIBRARIES} ${SQUIRREL_INCLUDE_DIRS}")
endif()

# CMake can't do aliases on imported target, so some hackery
# add_library(Squirrel::Squirrel ALIAS PkgConfig::SQUIRREL)
add_library(Squirrel::Squirrel INTERFACE IMPORTED)
foreach(name
    INTERFACE_LINK_LIBRARIES
    INTERFACE_INCLUDE_DIRECTORIES
    INTERFACE_COMPILE_DEFINITIONS
    INTERFACE_COMPILE_OPTIONS)
  get_property(value TARGET PkgConfig::SQUIRREL PROPERTY ${name} )
  set_property(TARGET Squirrel::Squirrel PROPERTY ${name} ${value})
endforeach()

find_package_handle_standard_args(Squirrel
  REQUIRED_VARS SQUIRREL_LIBRARIES SQUIRREL_INCLUDE_DIRS
  VERSION_VAR SQUIRREL_VERSION)

# EOF #
