# yaini - Yet another INI parser
# Copyright (C) 2008-2022 Ingo Ruhnke <grumbel@gmail.com>
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

cmake_minimum_required(VERSION 3.0)
project(yaini)

set(TINYCMMC_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/external/tinycmmc/modules/")
find_package(tinycmmc CONFIG)
message(STATUS "tinycmmc module path: ${TINYCMMC_MODULE_PATH}")
list(APPEND CMAKE_MODULE_PATH ${TINYCMMC_MODULE_PATH})

include(GNUInstallDirs)
include(MaximumWarnings)
include(ClangTidy)
include(GetProjectVersion)

# Build dependencies
function(build_dependencies)
  set(BUILD_TESTS OFF)
endfunction()
build_dependencies()

file(GLOB YAINI_SOURCES src/*.cpp)
file(GLOB YAINI_HEADER_SOURCES include/yaini/*.hpp)
add_library(yaini STATIC ${YAINI_SOURCES})
set_target_properties(yaini PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF)
set_target_properties(yaini PROPERTIES PUBLIC_HEADER "${YAINI_HEADER_SOURCES}")
target_compile_options(yaini PRIVATE ${WARNINGS_CXX_FLAGS})
target_include_directories(yaini PRIVATE include/yaini/)
target_include_directories(yaini SYSTEM PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
  $<INSTALL_INTERFACE:include>)

# file(GLOB YAINI_UTIL_SOURCES extra/*.cpp)
# add_executable(yaini-util ${YAINI_UTIL_SOURCES})
# set_target_properties(yaini-util PROPERTIES
#   CXX_STANDARD 20
#   CXX_STANDARD_REQUIRED ON
#   CXX_EXTENSIONS OFF)
# target_include_directories(yaini-util PRIVATE src/)
# target_link_libraries(yaini-util yaini)

if(BUILD_TESTS)
  find_package(GTest REQUIRED)

  # build yaini tests
  file(GLOB TEST_YAINI_SOURCES test/*.cpp)
  add_executable(test_yaini ${TEST_YAINI_SOURCES})
  set_target_properties(test_yaini PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)
  target_compile_options(test_yaini PRIVATE ${WARNINGS_CXX_FLAGS})
  target_include_directories(test_yaini PUBLIC src/)
  target_link_libraries(test_yaini
    GTest::GTest
    GTest::Main
    yaini)

  # add 'make test' target, use 'make test ARGS="-V"' or 'ctest -V' for verbose
  enable_testing()
  add_test(NAME test_yaini
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND test_yaini)
endif()

include(ExportAndInstallLibrary)

# EOF #
