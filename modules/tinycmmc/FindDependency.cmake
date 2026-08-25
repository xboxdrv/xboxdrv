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

macro(tinycmmc_find_dependency _NAME)
  find_package(${_NAME} QUIET)
  if(${${_NAME}_FOUND})
    message(STATUS "Found ${_NAME}: ${${_NAME}_DIR}")
  else()
    message(STATUS "Package ${_NAME} not found, trying external/${_NAME}")

    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/${_NAME}/CMakeLists.txt")
      message(FATAL_ERROR
        "The git submodule \"external/${_NAME}\" could not be found. "
        "To retrieve it, run:\n"
        "    git submodule update --init --recursive\n")
    else()
      set(BUILD_TESTS OFF)
      add_subdirectory(external/${_NAME} EXCLUDE_FROM_ALL)
      message(STATUS "Found ${_NAME}: external/${_NAME}")
    endif()
  endif()
endmacro()

# EOF #
