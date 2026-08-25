# uinpp - Linux uinput library for C++
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
project(uinpp VERSION 0.1.0)

set(TINYCMMC_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/external/cmake-modules/modules/")
find_package(tinycmmc CONFIG)
message(STATUS "tinycmmc module path: ${TINYCMMC_MODULE_PATH}")
list(APPEND CMAKE_MODULE_PATH ${TINYCMMC_MODULE_PATH})

include(GNUInstallDirs)
include(MaximumWarnings)
include(ClangTidy)

list(APPEND WARNINGS_CXX_FLAGS
  -Wno-stringop-overread # produces bogus warnings
  )

# Build dependencies
function(build_dependencies)
  set(BUILD_TESTS OFF)

  find_package(strut QUIET)
  if(NOT TARGET strut::strut)
    add_subdirectory(external/strutcpp/ EXCLUDE_FROM_ALL)
  endif()

  find_package(logmich QUIET)
  if(NOT TARGET logmich::logmich)
    add_subdirectory(external/logmich/ EXCLUDE_FROM_ALL)
  endif()
endfunction()
build_dependencies()

file(GLOB UINPP_SOURCES src/*.cpp)
file(GLOB UINPP_HEADER_SOURCES include/uinpp/*.hpp)
add_library(uinpp STATIC ${UINPP_SOURCES})
set_target_properties(uinpp PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF)
set_target_properties(uinpp PROPERTIES PUBLIC_HEADER "${UINPP_HEADER_SOURCES}")
target_compile_options(uinpp PRIVATE ${WARNINGS_CXX_FLAGS})
target_link_libraries(uinpp PUBLIC
  logmich::logmich
  )
target_include_directories(uinpp SYSTEM PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/uinpp>
  $<INSTALL_INTERFACE:include>)

file(GLOB UINPP_UTIL_SOURCES extra/*.cpp)
add_executable(uinpp-util ${UINPP_UTIL_SOURCES})
set_target_properties(uinpp-util PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF)
target_include_directories(uinpp-util PRIVATE src/)
target_link_libraries(uinpp-util uinpp)

if(BUILD_TESTS)
  find_package(GTest REQUIRED)

  # build uinpp tests
  file(GLOB TEST_UINPP_SOURCES tests/*.cpp)
  add_executable(test_uinpp ${TEST_UINPP_SOURCES})
  set_target_properties(test_uinpp PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)
  target_compile_options(test_uinpp PRIVATE ${WARNINGS_CXX_FLAGS})
  target_include_directories(test_uinpp PUBLIC src/)
  target_link_libraries(test_uinpp
    GTest::GTest
    GTest::Main
    uinpp)

  # add 'make test' target, use 'make test ARGS="-V"' or 'ctest -V' for verbose
  enable_testing()
  add_test(NAME test_uinpp
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND test_uinpp)
endif()

include(ExportAndInstallLibrary)

# EOF #
