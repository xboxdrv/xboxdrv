# tinycmmc - Tiny CMake module collection
# Copyright (C) 2022 Ingo Ruhnke <grumbel@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Bootstrap file that is looking for where tinycmmc is installed

find_package(tinycmmc CONFIG)
if(tinycmmc_FOUND)
  message(STATUS "tinycmmc module path: ${TINYCMMC_MODULE_PATH}")
  list(APPEND CMAKE_MODULE_PATH ${TINYCMMC_MODULE_PATH})
else()
  if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/tinycmmc/CMakeLists.txt")
    message(FATAL_ERROR
      "The git submodule \"external/tinycmmc\" could not be found. "
      "To retrieve it, run:\n"
      "    git submodule update --init --recursive\n")
  else()
    set(TINYCMMC_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/tinycmmc/modules/")
    message(STATUS "tinycmmc module path: ${TINYCMMC_MODULE_PATH}")
    list(APPEND CMAKE_MODULE_PATH "${TINYCMMC_MODULE_PATH}")
  endif()
endif()

include(TinyCMMC)

# EOF #
