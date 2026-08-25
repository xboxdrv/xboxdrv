# strut - Collection of string utilities
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
project(strut VERSION 0.0.0)

set(TINYCMMC_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/external/tinycmmc/modules/")
find_package(tinycmmc CONFIG)
message(STATUS "tinycmmc module path: ${TINYCMMC_MODULE_PATH}")
list(APPEND CMAKE_MODULE_PATH ${TINYCMMC_MODULE_PATH})

include(GNUInstallDirs)
include(MaximumWarnings)
include(ClangTidy)

file(GLOB STRUT_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  include/strut/*.hpp)

file(GLOB STRUT_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  src/*.cpp)

add_library(strut STATIC ${STRUT_SOURCES})
set_target_properties(strut PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF)
target_include_directories(strut INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>/)
target_include_directories(strut PRIVATE include/strut/)
set_target_properties(strut PROPERTIES PUBLIC_HEADER "${STRUT_HEADERS}")

if(BUILD_TESTS)
  find_package(GTest REQUIRED)

  file(GLOB TEST_STRUT_SOURCES test/*_test.cpp)
  add_executable(test_strut ${TEST_STRUT_SOURCES})
  set_target_properties(test_strut PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)
  target_compile_options(test_strut PRIVATE ${WARNINGS_CXX_FLAGS})
  target_link_libraries(test_strut
    strut
    GTest::GTest
    GTest::Main)

  enable_testing()
  add_test(NAME test_strut
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMAND test_strut)
endif()

include(ExportAndInstallLibrary)

# EOF #
