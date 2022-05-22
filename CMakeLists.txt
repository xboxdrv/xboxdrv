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

cmake_minimum_required(VERSION 3.14)
project(xboxdrv)

option(BUILD_TESTS "Build tests" OFF)
option(WARNINGS "Switch on extra warnings" OFF)
option(WERROR "Turn warnings into errors" OFF)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(WARNINGS)
  set(WARNINGS_CXX_FLAGS ${WARNINGS_CXX_FLAGS}
    -pedantic -Wall -Wextra -Wno-c++0x-compat -Wnon-virtual-dtor -Weffc++
    -Wshadow -Wcast-qual -Winit-self -Wno-unused-parameter
    -Wconversion -Wno-sign-conversion)
  # -Winline -Wfloat-equal -Wunreachable-code
endif()

if(WERROR)
  set(WARNINGS_CXX_FLAGS ${WARNINGS_CXX_FLAGS}
    -Werror)
endif()

set(XBOXDRV_BINDIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "Install location for executables")
set(XBOXDRV_MANDIR "${CMAKE_INSTALL_PREFIX}/share/man" CACHE PATH "Install location for datafiles")

find_package(Threads REQUIRED)
find_package(X11 REQUIRED)

find_package(PkgConfig)
find_package(Python REQUIRED)
find_package(fmt REQUIRED)
pkg_search_module(GTK REQUIRED gtk+-3.0 IMPORTED_TARGET)
pkg_search_module(USB REQUIRED libusb-1.0 IMPORTED_TARGET)
pkg_search_module(UDEV REQUIRED libudev IMPORTED_TARGET)
pkg_search_module(DBUSGLIB REQUIRED dbus-glib-1 IMPORTED_TARGET)
pkg_search_module(CWIID OPTIONAL cwiid IMPORTED_TARGET)
# FIXME: define -DHAVE_CWIID when found

include_directories(src/)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/src/)

file(GLOB XBOXDRV_SOURCES_CXX RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  src/*.cpp
  src/axisevent/*.cpp
  src/axisfilter/*.cpp
  src/buttonevent/*.cpp
  src/buttonfilter/*.cpp
  src/controller/*.cpp
  src/modifier/*.cpp
  src/symbols/*.cpp
  src/ini/*.cpp
  src/uinput/*.cpp
  src/util/*.cpp)

set(XBOXDRV_SOURCES_CXX
  ${XBOXDRV_SOURCES_CXX}
  ${CMAKE_CURRENT_BINARY_DIR}/src/xboxdrv_controller_glue.hpp
  ${CMAKE_CURRENT_BINARY_DIR}/src/xboxdrv_daemon_glue.hpp
  ${CMAKE_CURRENT_BINARY_DIR}/src/xboxdrv_vfs.hpp)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/src)

set(DBUS_GLUE ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/dbus_glue.py)
set(BIN2H ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/bin2h.py)

set(TARGET ${CMAKE_CURRENT_BINARY_DIR}/src/xboxdrv_vfs.hpp)
set(SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/examples/mouse.xboxdrv
  ${CMAKE_CURRENT_SOURCE_DIR}/examples/xpad-wireless.xboxdrv)
add_custom_command(
  OUTPUT ${TARGET}
  DEPENDS ${SOURCES}
  COMMAND ${BIN2H} --output ${TARGET} --prefix ${CMAKE_CURRENT_SOURCE_DIR} --namespace=xboxdrv_vfs ${SOURCES})

set(TARGET ${CMAKE_CURRENT_BINARY_DIR}/src/xboxdrv_controller_glue.hpp)
set(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/xboxdrv_controller.xml)
add_custom_command(
  OUTPUT ${TARGET}
  DEPENDS ${SOURCE}
  COMMAND ${DBUS_GLUE} -o ${TARGET} ${SOURCE} --dbus-prefix=xboxdrv_controller)

set(TARGET ${CMAKE_CURRENT_BINARY_DIR}/src/xboxdrv_daemon_glue.hpp)
set(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/xboxdrv_daemon.xml)
add_custom_command(
  OUTPUT ${TARGET}
  DEPENDS ${SOURCE}
  COMMAND ${DBUS_GLUE} -o ${TARGET} ${SOURCE} --dbus-prefix=xboxdrv_daemon)

function(build_dependencies)
  set(BUILD_TESTS OFF)
  find_package(argparser)
  if(NOT TARGET argparser::argparser)
    add_subdirectory(external/argparser/)
  endif()

  find_package(uinpp)
  if(NOT TARGET uinpp::uinpp)
    add_subdirectory(external/uinpp/)
  endif()
endfunction()
build_dependencies()

add_library(libxboxdrv STATIC ${XBOXDRV_SOURCES_CXX})
set_target_properties(libxboxdrv PROPERTIES OUTPUT_NAME xboxdrv)
set_target_properties(libxboxdrv PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF)

file(STRINGS VERSION XBOXDRV_VERSION)
target_compile_definitions(libxboxdrv PUBLIC -DPACKAGE_VERSION="${XBOXDRV_VERSION}")
target_compile_options(libxboxdrv PRIVATE ${WARNINGS_CXX_FLAGS})

target_link_libraries(libxboxdrv PUBLIC
  argparser
  uinpp::uinpp
  fmt::fmt
  PkgConfig::DBUSGLIB
  PkgConfig::USB
  PkgConfig::UDEV
  X11::X11)

add_executable(xboxdrv src/main/main.cpp)
set_target_properties(xboxdrv PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF)
target_link_libraries(xboxdrv libxboxdrv)

if(BUILD_TESTS)
  enable_testing()

  find_package(GTest REQUIRED)

  # build automatic tests
  file(GLOB XBOXDRV_TEST_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    test/*_gtest.cpp)
  add_executable(test_xboxdrv ${XBOXDRV_TEST_SOURCES})
  target_compile_options(test_xboxdrv PRIVATE ${WARNINGS_CXX_FLAGS})
  target_link_libraries(test_xboxdrv GTest::GTest GTest::Main libxboxdrv)

  add_test(NAME test_xboxdrv
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND test_xboxdrv)

  # build old manual tests
  file(GLOB TEST_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    test/*_test.cpp)
  foreach(SOURCE ${TEST_SOURCES})
    get_filename_component(SOURCE_BASENAME ${SOURCE} NAME_WE)
    add_executable(${SOURCE_BASENAME} ${SOURCE})
    target_link_libraries(${SOURCE_BASENAME} libxboxdrv)
  endforeach(SOURCE)
endif()

install(
  TARGETS xboxdrv
  RUNTIME DESTINATION ${XBOXDRV_BINDIR})
install(
  PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/xboxdrvctl
  DESTINATION ${XBOXDRV_BINDIR})

# EOF #
