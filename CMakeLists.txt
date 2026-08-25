# LogMich - A Trivial Logging Library
# Copyright (C) 2020 Ingo Ruhnke <grumbel@gmail.com>
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
project(logmich VERSION 0.2.0)

# CMake modules
set(TINYCMMC_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/external/cmake-modules/modules/")
find_package(tinycmmc CONFIG)
message(STATUS "tinycmmc module path: ${TINYCMMC_MODULE_PATH}")
list(APPEND CMAKE_MODULE_PATH ${TINYCMMC_MODULE_PATH})

include(GNUInstallDirs)
include(ClangTidy)
include(MaximumWarnings)

# Dependencies
find_package(fmt REQUIRED)

# Build tinycmmc
set(TINYCMMC_HEADERS
  include/logmich/logger.hpp
  include/logmich/log.hpp)
add_library(logmich STATIC
  src/logger.cpp
  src/log.cpp)
set_target_properties(logmich PROPERTIES
  PUBLIC_HEADER "${TINYCMMC_HEADERS}")
set_target_properties(logmich PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF)
target_compile_options(logmich PUBLIC ${WARNINGS_CXX_FLAGS})
target_include_directories(logmich PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(logmich PUBLIC fmt::fmt)

if(BUILD_TESTS)
  add_executable(test_logmich test/test_logmich.cpp)
  set_target_properties(test_logmich PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)
  target_link_libraries(test_logmich logmich)

  enable_testing()
  add_test(NAME test_logmich
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_logmich.sh
    ${CMAKE_CURRENT_BINARY_DIR}/test_logmich
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_logmich.expected
    ${CMAKE_CURRENT_SOURCE_DIR})
endif()

include(ExportAndInstallLibrary)

# EOF #
