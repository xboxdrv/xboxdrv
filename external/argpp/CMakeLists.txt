# ArgParse - A Command Line Argument Parser for C++
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

cmake_minimum_required(VERSION 3.10)
project(argpp VERSION 1.0.0)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(BUILD_TESTS "Build tests" OFF)
option(WARNINGS "Enable extra compiler warnings" OFF)
option(WERROR "Treat warnings as errors" OFF)
option(CLANG_TIDY "Enable clang-tidy" OFF)
option(CLANG_TIDY_FIX "Apply clang-tidy fixes to source code" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Optional extra warnings (replaces tinycmmc MaximumWarnings)
set(WARNINGS_CXX_FLAGS "")
if(WERROR)
  list(APPEND WARNINGS_CXX_FLAGS -Werror)
endif()
if(WARNINGS)
  list(APPEND WARNINGS_CXX_FLAGS -Wall -Wextra -Wpedantic)
endif()

# Optional clang-tidy (replaces tinycmmc ClangTidy)
if(CLANG_TIDY AND CMAKE_VERSION VERSION_GREATER_EQUAL 3.6)
  find_program(CLANG_TIDY_EXE NAMES clang-tidy DOC "Path to clang-tidy executable")
  if(NOT CLANG_TIDY_EXE)
    message(FATAL_ERROR "clang-tidy not found.")
  endif()
  message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
  if(CLANG_TIDY_FIX)
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}" "-fix")
  else()
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
  endif()
endif()

add_library(argpp STATIC
  src/parser.cpp
  src/prettyprinter.cpp)
# Public BUILD_INTERFACE needs both roots:
#   include/       → #include <argpp/argpp.hpp>
#   include/argpp/ → #include <argpp.hpp> / "argpp.hpp"
# Install layout is include/argpp/argpp.hpp, so INSTALL_INTERFACE is include/.
target_include_directories(argpp PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/argpp>
  $<INSTALL_INTERFACE:include>)
target_include_directories(argpp PRIVATE
  src/)
target_compile_options(argpp PRIVATE ${WARNINGS_CXX_FLAGS})
set_target_properties(argpp PROPERTIES
  PUBLIC_HEADER "include/argpp/argpp.hpp"
  EXPORT_NAME argpp)

add_library(argpp::argpp ALIAS argpp)

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

# Install & package config (replaces tinycmmc ExportAndInstallLibrary)
install(TARGETS argpp
  EXPORT argpp
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

configure_package_config_file(
  argpp-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/argpp-config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/argpp)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/argpp-config-version.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMinorVersion)

export(EXPORT argpp
  NAMESPACE argpp::
  FILE ${CMAKE_CURRENT_BINARY_DIR}/argpp-targets.cmake)

install(EXPORT argpp
  FILE argpp-targets.cmake
  NAMESPACE argpp::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/argpp)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/argpp-config.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/argpp-config-version.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/argpp)

# EOF #
