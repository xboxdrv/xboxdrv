# Xbox360 USB Gamepad Userspace Driver
# Copyright (C) 2015-2026 Ingo Ruhnke <grumbel@gmail.com>
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

cmake_minimum_required(VERSION 3.14)

# Source of truth: top-level VERSION (e.g. "0.9.0-dev"), or
# -DPROJECT_VERSION_FULL=... from packaging (Nix passes the full
# "0.9.0-dev.<revCount>+g<shortRev>" string when building from a flake).
if(NOT DEFINED PROJECT_VERSION_FULL)
  file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" PROJECT_VERSION_FULL LIMIT_COUNT 1)
endif()
string(STRIP "${PROJECT_VERSION_FULL}" PROJECT_VERSION_FULL)

# Development builds: append .<revCount>+g<shortRev>[-dirty] when VERSION
# ends in -dev and packaging has not already supplied the full form.
if(PROJECT_VERSION_FULL MATCHES "-dev$" AND NOT PROJECT_VERSION_FULL MATCHES "\\+g")
  find_package(Git QUIET)
  if(Git_FOUND)
    execute_process(
      COMMAND ${GIT_EXECUTABLE} -C "${CMAKE_CURRENT_SOURCE_DIR}" rev-list --count HEAD
      OUTPUT_VARIABLE _xboxdrv_rev_count
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET)
    execute_process(
      COMMAND ${GIT_EXECUTABLE} -C "${CMAKE_CURRENT_SOURCE_DIR}" rev-parse --short=7 HEAD
      OUTPUT_VARIABLE _xboxdrv_short_rev
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET)
    execute_process(
      COMMAND ${GIT_EXECUTABLE} -C "${CMAKE_CURRENT_SOURCE_DIR}" status --porcelain
      OUTPUT_VARIABLE _xboxdrv_git_status
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET)
    if(_xboxdrv_rev_count STREQUAL "")
      set(_xboxdrv_rev_count "0")
    endif()
    if(_xboxdrv_short_rev STREQUAL "")
      set(_xboxdrv_short_rev "dirty")
    endif()
    set(_xboxdrv_dirty "")
    if(NOT _xboxdrv_git_status STREQUAL "")
      set(_xboxdrv_dirty "-dirty")
    endif()
    set(PROJECT_VERSION_FULL
      "${PROJECT_VERSION_FULL}.${_xboxdrv_rev_count}+g${_xboxdrv_short_rev}${_xboxdrv_dirty}")
  endif()
endif()

# project() needs a numeric version; strip -dev / +git / similar suffixes.
string(REGEX MATCH "^[0-9]+(\\.[0-9]+)*" _project_version_cmake "${PROJECT_VERSION_FULL}")
if(NOT _project_version_cmake)
  set(_project_version_cmake "0.0.0")
endif()

project(xboxdrv VERSION ${_project_version_cmake} LANGUAGES CXX C)
message(STATUS "Project version: ${PROJECT_VERSION_FULL} (CMake ${PROJECT_VERSION})")

set(TINYCMMC_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/external/tinycmmc/modules/")
find_package(tinycmmc CONFIG QUIET)
message(STATUS "tinycmmc module path: ${TINYCMMC_MODULE_PATH}")
list(APPEND CMAKE_MODULE_PATH ${TINYCMMC_MODULE_PATH})

option(BUILD_TESTS "Build tests" OFF)

include(GNUInstallDirs)
include(MaximumWarnings)
include(ClangTidy)

# -Wpacked is noisy: uint8_t-only structs legitimately need no packing, and
# bitfield USB report layouts still use packed where it matters. Drop the
# warning and force -Wno-packed so -Werror cannot revive it.
list(REMOVE_ITEM WARNINGS_CXX_FLAGS -Wpacked)
list(APPEND WARNINGS_CXX_FLAGS -Wno-packed)

find_package(Threads REQUIRED)
find_package(X11 REQUIRED)

find_package(PkgConfig)
find_package(Python REQUIRED)
pkg_search_module(USB REQUIRED libusb-1.0 IMPORTED_TARGET)
pkg_search_module(UDEV REQUIRED libudev IMPORTED_TARGET)
pkg_search_module(LIBEVDEV REQUIRED libevdev IMPORTED_TARGET)
pkg_search_module(GIO REQUIRED gio-2.0 IMPORTED_TARGET)
pkg_search_module(CWIID cwiid IMPORTED_TARGET)
pkg_search_module(BLUEZ bluez IMPORTED_TARGET)
pkg_search_module(PIPEWIRE libpipewire-0.3 IMPORTED_TARGET)
if(PIPEWIRE_FOUND)
  set(HAVE_PIPEWIRE TRUE)
  message(STATUS "libpipewire-0.3 found — --headset-pulse enabled")
else()
  set(HAVE_PIPEWIRE FALSE)
  message(STATUS "libpipewire-0.3 not found — --headset-pulse disabled at build time")
endif()
if(CWIID_FOUND AND BLUEZ_FOUND)
  set(HAVE_CWIID TRUE)
  message(STATUS "cwiid + bluez found — Wiimote support enabled")
elseif(CWIID_FOUND AND NOT BLUEZ_FOUND)
  set(HAVE_CWIID FALSE)
  message(STATUS "cwiid found but bluez missing — Wiimote support disabled")
else()
  set(HAVE_CWIID FALSE)
  message(STATUS "cwiid not found — Wiimote support disabled")
endif()

# Prefer vendored external/ trees. find_package() alone is unsafe here:
# a previous install (e.g. nix store xboxdrv that exported logmich headers
# still using {fmt}) can satisfy the package name and pull stale includes.
function(xboxdrv_add_external name subdir)
  if(TARGET ${name}::${name})
    message(STATUS "Using existing target ${name}::${name}")
    return()
  endif()
  set(_path "${CMAKE_CURRENT_SOURCE_DIR}/external/${subdir}")
  if(EXISTS "${_path}/CMakeLists.txt")
    message(STATUS "Building vendored ${name} from external/${subdir}")
    add_subdirectory("${_path}" EXCLUDE_FROM_ALL)
  else()
    find_package(${name} QUIET)
    if(NOT TARGET ${name}::${name})
      message(FATAL_ERROR
        "${name} not found: need external/${subdir} or an installed ${name} package")
    endif()
    message(STATUS "Using installed ${name} (no external/${subdir})")
  endif()
endfunction()

function(build_dependencies)
  set(BUILD_TESTS OFF)
  xboxdrv_add_external(argpp argpp)
  xboxdrv_add_external(logmich logmich)
  xboxdrv_add_external(strut strutcpp)
  xboxdrv_add_external(uinpp uinpp)
  xboxdrv_add_external(yaini yaini)
  xboxdrv_add_external(unsebu unsebu)
endfunction()
build_dependencies()

file(GLOB XBOXDRV_SOURCES_CXX RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  src/*.cpp
  src/axisevent/*.cpp
  src/axisfilter/*.cpp
  src/buttonevent/*.cpp
  src/buttonfilter/*.cpp
  src/controller/*.cpp
  src/modifier/*.cpp
  src/uinput/*.cpp
  src/util/*.cpp)

set(G72X_SOURCES
  external/g72x/g72x.c
  external/g72x/g721.c
  external/g72x/g711.c)

list(APPEND XBOXDRV_SOURCES_CXX
  ${CMAKE_CURRENT_BINARY_DIR}/src/xboxdrv_vfs.hpp)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/src)

set(BIN2H ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/bin2h.py)

set(TARGET ${CMAKE_CURRENT_BINARY_DIR}/src/xboxdrv_vfs.hpp)
set(SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/examples/mouse.xboxdrv
  ${CMAKE_CURRENT_SOURCE_DIR}/examples/xpad-wireless.xboxdrv)
add_custom_command(
  OUTPUT ${TARGET}
  DEPENDS ${SOURCES}
  COMMAND ${BIN2H} --output ${TARGET} --prefix ${CMAKE_CURRENT_SOURCE_DIR} --namespace=xboxdrv_vfs ${SOURCES})

# Vendored Sun CCITT G.72x reference: isolate so project -Werror / C++-only
# flags never reach this ancient C code.
add_library(g72x STATIC ${G72X_SOURCES})
set_target_properties(g72x PROPERTIES
  C_STANDARD 90
  C_STANDARD_REQUIRED OFF
  C_EXTENSIONS ON
  POSITION_INDEPENDENT_CODE ON)
target_include_directories(g72x PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/external/g72x)
target_compile_options(g72x PRIVATE -w)

add_library(libxboxdrv STATIC ${XBOXDRV_SOURCES_CXX})
set_target_properties(libxboxdrv PROPERTIES OUTPUT_NAME xboxdrv)
set_target_properties(libxboxdrv PROPERTIES
  CXX_STANDARD 23
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF)
target_include_directories(libxboxdrv PUBLIC src/ ${CMAKE_CURRENT_BINARY_DIR}/src/)
# Isolate PACKAGE_VERSION to one generated header included only by
# xboxdrv_version.cpp. Write only when content changes so a version/git-rev
# bump does not force a full libxboxdrv rebuild (and does not touch a global
# -D that could invalidate more of the ninja graph than needed).
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/src")
set(_xboxdrv_version_def "${CMAKE_CURRENT_BINARY_DIR}/src/xboxdrv_version_def.hpp")
set(_xboxdrv_version_def_content
  "#pragma once\n#define PACKAGE_VERSION \"${PROJECT_VERSION_FULL}\"\n")
if(EXISTS "${_xboxdrv_version_def}")
  file(READ "${_xboxdrv_version_def}" _xboxdrv_version_def_old)
else()
  set(_xboxdrv_version_def_old "")
endif()
if(NOT "${_xboxdrv_version_def_old}" STREQUAL "${_xboxdrv_version_def_content}")
  file(WRITE "${_xboxdrv_version_def}" "${_xboxdrv_version_def_content}")
endif()
# Explicit dep so Ninja rebuilds the one TU when the header changes.
set_source_files_properties(src/xboxdrv_version.cpp PROPERTIES
  OBJECT_DEPENDS "${_xboxdrv_version_def}")
if(HAVE_CWIID)
  target_compile_definitions(libxboxdrv PUBLIC -DHAVE_CWIID)
endif()
if(HAVE_PIPEWIRE)
  target_compile_definitions(libxboxdrv PUBLIC -DHAVE_PIPEWIRE)
endif()
target_compile_options(libxboxdrv PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${WARNINGS_CXX_FLAGS}>)

target_link_libraries(libxboxdrv PUBLIC
  g72x
  argpp::argpp
  strut::strut
  uinpp::uinpp
  unsebu::unsebu
  yaini::yaini
  PkgConfig::GIO
  PkgConfig::USB
  PkgConfig::UDEV
  PkgConfig::LIBEVDEV
  X11::X11)
if(HAVE_CWIID)
  target_link_libraries(libxboxdrv PUBLIC PkgConfig::CWIID PkgConfig::BLUEZ)
endif()
if(HAVE_PIPEWIRE)
  target_sources(libxboxdrv PRIVATE src/headset_pipewire.cpp)
  target_link_libraries(libxboxdrv PUBLIC PkgConfig::PIPEWIRE)
endif()

add_executable(xboxdrv src/main/main.cpp)
set_target_properties(xboxdrv PROPERTIES
  CXX_STANDARD 23
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF)
target_link_libraries(xboxdrv libxboxdrv)

if(BUILD_TESTS)
  enable_testing()

  # find_package(GTest REQUIRED)
  #
  # build automatic tests
  # file(GLOB XBOXDRV_TEST_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  #   test/*_gtest.cpp)
  # add_executable(test_xboxdrv ${XBOXDRV_TEST_SOURCES})
  # set_target_properties(test_xboxdrv PROPERTIES
  #   CXX_STANDARD 23
  #   CXX_STANDARD_REQUIRED ON
  #   CXX_EXTENSIONS OFF)
  # target_compile_options(test_xboxdrv PRIVATE ${WARNINGS_CXX_FLAGS})
  # target_link_libraries(test_xboxdrv GTest::GTest GTest::Main libxboxdrv)
  #
  # add_test(NAME test_xboxdrv
  #   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  #   COMMAND test_xboxdrv)

  # build old manual tests
  file(GLOB TEST_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    test/*_test.cpp)
  foreach(SOURCE ${TEST_SOURCES})
    get_filename_component(SOURCE_BASENAME ${SOURCE} NAME_WE)
    add_executable(${SOURCE_BASENAME} ${SOURCE})
    set_target_properties(${SOURCE_BASENAME} PROPERTIES
      CXX_STANDARD 23
      CXX_STANDARD_REQUIRED ON
      CXX_EXTENSIONS OFF)
    target_compile_options(${SOURCE_BASENAME} PRIVATE ${WARNINGS_CXX_FLAGS})
    target_link_libraries(${SOURCE_BASENAME} libxboxdrv)
  endforeach(SOURCE)
endif()

install(TARGETS xboxdrv
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/xboxdrvctl
  DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES doc/xboxdrv.1
  DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)

# EOF #
