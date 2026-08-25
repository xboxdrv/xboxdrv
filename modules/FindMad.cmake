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

pkg_search_module(MAD mad IMPORTED_TARGET)
if(NOT MAD_FOUND)
  find_library(MAD_LIBRARIES NAMES "mad" REQUIRED)
  find_path(MAD_INCLUDE_DIRS NAMES "mad.h" PATH_SUFFIXES "/" "mad/")
  if (NOT MAD_INCLUDE_DIRS)
    message(FATAL_ERROR "mad not found")
  endif()

  add_library(PkgConfig::MAD INTERFACE IMPORTED)
  target_link_libraries(PkgConfig::MAD INTERFACE ${MAD_LIBRARIES})
  target_include_directories(PkgConfig::MAD INTERFACE ${MAD_INCLUDE_DIRS})
  message(STATUS "Found Mad: ${MAD_LIBRARIES} ${MAD_INCLUDE_DIRS}")
endif()

# CMake can't do aliases on imported target, so some hackery
# add_library(Mad::Mad ALIAS PkgConfig::MAD)
add_library(Mad::Mad INTERFACE IMPORTED)
foreach(name
    INTERFACE_LINK_LIBRARIES
    INTERFACE_INCLUDE_DIRECTORIES
    INTERFACE_COMPILE_DEFINITIONS
    INTERFACE_COMPILE_OPTIONS)
  get_property(value TARGET PkgConfig::MAD PROPERTY ${name} )
  set_property(TARGET Mad::Mad PROPERTY ${name} ${value})
endforeach()

find_package_handle_standard_args(Mad
  REQUIRED_VARS MAD_LIBRARIES MAD_INCLUDE_DIRS
  VERSION_VAR MAD_VERSION)

# EOF #
