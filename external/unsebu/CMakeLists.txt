# unsebu - libusb helper for C++
# Copyright (C) 2020-2026 Ingo Ruhnke <grumbel@gmail.com>
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

cmake_minimum_required(VERSION 3.10)
project(unsebu VERSION 0.1.0)

include(mk/cmake/TinyCMMC.cmake)

find_package(PkgConfig)
pkg_search_module(USB REQUIRED libusb-1.0 IMPORTED_TARGET)
pkg_search_module(UDEV REQUIRED libudev IMPORTED_TARGET)
pkg_search_module(GIO REQUIRED gio-2.0 IMPORTED_TARGET)

tinycmmc_find_dependency(logmich)

file(GLOB UNSEBU_HEADER_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  include/unsebu/*.hpp)
file(GLOB UNSEBU_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  src/*.cpp)

add_library(unsebu STATIC ${UNSEBU_SOURCES})
set_target_properties(unsebu PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF)
set_target_properties(unsebu PROPERTIES PUBLIC_HEADER "${UNSEBU_HEADER_SOURCES}")
target_compile_options(unsebu PRIVATE ${TINYCMMC_WARNINGS_CXX_FLAGS})
# Public: include/ so consumers can #include <unsebu/usb_helper.hpp>
# Private: include/unsebu so the library sources can #include "usb_helper.hpp"
target_include_directories(unsebu PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_include_directories(unsebu PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include/unsebu)
target_link_libraries(unsebu PUBLIC
  logmich::logmich
  PkgConfig::GIO
  PkgConfig::USB
  PkgConfig::UDEV)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  tinycmmc_export_and_install_library(unsebu)
else()
  add_library(unsebu::unsebu ALIAS unsebu)
endif()

# EOF #
