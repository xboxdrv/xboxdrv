# unsebu - libusb helper for C++
# Copyright (C) 2020-2022 Ingo Ruhnke <grumbel@gmail.com>
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

cmake_minimum_required(VERSION 3.6)
project(unsebu VERSION 0.1.0)

include(mk/cmake/TinyCMMC.cmake)

find_package(PkgConfig)
find_package(fmt REQUIRED)
pkg_search_module(USB REQUIRED libusb-1.0 IMPORTED_TARGET)
pkg_search_module(UDEV REQUIRED libudev IMPORTED_TARGET)
pkg_search_module(DBUSGLIB REQUIRED dbus-glib-1 IMPORTED_TARGET)

tinycmmc_find_dependency(logmich)

file(GLOB UNSEBU_HEADER_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  include/unsebu/*.hpp)
file(GLOB UNSEBU_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  src/*.cpp)

add_library(unsebu STATIC ${UNSEBU_SOURCES})
set_target_properties(unsebu PROPERTIES PUBLIC_HEADER "${UNSEBU_HEADER_SOURCES}")
target_compile_options(unsebu PRIVATE ${TINYCMMC_WARNINGS_CXX_FLAGS})
target_include_directories(unsebu PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/unsebu/>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(unsebu PUBLIC
  fmt::fmt
  PkgConfig::DBUSGLIB
  PkgConfig::USB
  PkgConfig::UDEV)

tinycmmc_export_and_install_library(unsebu)

# EOF #
