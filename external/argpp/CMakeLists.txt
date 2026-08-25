# Xbox360 USB Gamepad Userspace Driver
# Copyright (C) 2015 Ingo Ruhnke <grumbel@gmail.com>
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
project(argpp VERSION 1.0.0)

set(TINYCMMC_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/external/cmake-modules/modules/")
find_package(tinycmmc CONFIG)
message(STATUS "tinycmmc module path: ${TINYCMMC_MODULE_PATH}")
list(APPEND CMAKE_MODULE_PATH ${TINYCMMC_MODULE_PATH})

include(GNUInstallDirs)
include(MaximumWarnings)
include(ClangTidy)

option(BUILD_TESTS "Build tests" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(argpp STATIC
  src/parser.cpp
  src/prettyprinter.cpp)
target_include_directories(argpp PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/argpp/>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)
target_include_directories(argpp PRIVATE
  src/)
target_compile_options(argpp PRIVATE ${WARNINGS_CXX_FLAGS})
set_target_properties(argpp PROPERTIES PUBLIC_HEADER "include/argpp/argpp.hpp")

if(BUILD_TESTS)
  enable_testing()

  file(GLOB TEST_ARGPP_SOURCES test/*_test.cpp)

  foreach(SRC ${TEST_ARGPP_SOURCES})
    get_filename_component(SRC_BASENAME ${SRC} NAME_WE)
    add_executable(${SRC_BASENAME} ${SRC})
    target_link_libraries(${SRC_BASENAME} argpp)
    target_compile_options(${SRC_BASENAME} PRIVATE ${WARNINGS_CXX_FLAGS})
    target_include_directories(${SRC_BASENAME} PRIVATE src/)

    add_test(NAME ${SRC_BASENAME}
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
      COMMAND "${SRC_BASENAME}")
  endforeach()
endif()

include(ExportAndInstallLibrary)

# EOF #
