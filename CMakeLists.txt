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
project(xboxdrv)

option(BUILD_TESTS "Build tests" OFF)
option(WARNINGS "Switch on extra warnings" OFF)
option(WERROR "Turn warnings into errors" OFF)

include(GNUInstallDirs)

add_compile_options(-std=c++11)

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
find_package(X11)
find_package(GTK2 COMPONENTS gtk)

find_package(PkgConfig)
pkg_search_module(USB REQUIRED libusb-1.0)
pkg_search_module(UDEV REQUIRED libudev)
pkg_search_module(DBUSGLIB REQUIRED dbus-glib-1)
pkg_search_module(CWIID OPTIONAL cwiid)
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
  src/symbols/*.cpp)

set(XBOXDRV_SOURCES_CXX
  ${XBOXDRV_SOURCES_CXX}
  ${CMAKE_CURRENT_BINARY_DIR}/src/xboxdrv_controller_glue.hpp
  ${CMAKE_CURRENT_BINARY_DIR}/src/xboxdrv_daemon_glue.hpp
  ${CMAKE_CURRENT_BINARY_DIR}/src/xboxdrv_vfs.hpp)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/src)

set(DBUS_GLUE ${CMAKE_CURRENT_SOURCE_DIR}/dbus_glue.py)
set(BIN2H ${CMAKE_CURRENT_SOURCE_DIR}/bin2h.py)

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

add_library(libxboxdrv STATIC ${XBOXDRV_SOURCES_CXX})
set_target_properties(libxboxdrv PROPERTIES OUTPUT_NAME xboxdrv)

file(STRINGS VERSION XBOXDRV_VERSION)
target_compile_definitions(libxboxdrv PUBLIC -DPACKAGE_VERSION="${XBOXDRV_VERSION}")
target_compile_options(libxboxdrv PRIVATE ${WARNINGS_CXX_FLAGS})

target_include_directories(libxboxdrv SYSTEM PUBLIC
  ${DBUSGLIB_INCLUDE_DIRS}
  ${GLIB_INCLUDE_DIRS}
  ${GTHREAD_INCLUDE_DIRS}
  ${USB_INCLUDE_DIRS}
  ${UDEV_INCLUDE_DIRS}
  ${X11_INCLUDE_DIR})

target_link_libraries(libxboxdrv PUBLIC
  ${DBUSGLIB_LIBRARIES}
  ${GLIB_LIBRARIES}
  ${GTHREAD_LIBRARIES}
  ${USB_LIBRARIES}
  ${UDEV_LIBRARIES}
  ${X11_LIBRARIES})

add_executable(xboxdrv src/main/main.cpp)
target_link_libraries(xboxdrv libxboxdrv)

file(GLOB VIRTUALKEYBOARD_SOURCES_CXX RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  src/virtualkeyboard/*.cpp)
add_executable(virtualkeyboard ${VIRTUALKEYBOARD_SOURCES_CXX})
target_include_directories(virtualkeyboard SYSTEM PUBLIC
  ${CAIRO_INCLUDE_DIRS}
  ${DBUSGLIB_INCLUDE_DIRS}
  ${GLIB_INCLUDE_DIRS}
  ${GLIB_INCLUDE_DIRS}
  ${GTHREAD_INCLUDE_DIRS}
  ${GTK2_INCLUDE_DIRS}
  ${UDEV_INCLUDE_DIRS}
  ${USB_INCLUDE_DIRS}
  ${X11_INCLUDE_DIRS})
target_link_libraries(virtualkeyboard libxboxdrv)
target_link_libraries(virtualkeyboard
  ${CAIRO_LIBRARIES}
  ${DBUSGLIB_LIBRARIES}
  ${GLIB_LIBRARIES}
  ${GLIB_LIBRARIES}
  ${GTHREAD_LIBRARIES}
  ${GTK2_LIBRARIES}
  ${UDEV_LIBRARIES}
  ${USB_LIBRARIES}
  ${X11_LIBRARIES})

if(BUILD_TESTS)
  include_directories(SYSTEM external/googletest/googletest/include/)

  # build gtest
  add_library(gtest STATIC ${CMAKE_CURRENT_SOURCE_DIR}/external/googletest/googletest/src/gtest-all.cc)
  target_include_directories(gtest SYSTEM PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external/googletest/googletest/)

  # build gtest_main
  add_library(gtest_main STATIC ${CMAKE_CURRENT_SOURCE_DIR}/external/googletest/googletest/src/gtest_main.cc)

  # build automatic tests
  file(GLOB XBOXDRV_TEST_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    test/*_gtest.cpp)
  add_executable(test_xboxdrv ${XBOXDRV_TEST_SOURCES})
  target_compile_options(test_xboxdrv PRIVATE ${WARNINGS_CXX_FLAGS})
  target_link_libraries(test_xboxdrv gtest gtest_main libxboxdrv ${CMAKE_THREAD_LIBS_INIT})

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
  TARGETS virtualkeyboard
  RUNTIME DESTINATION ${XBOXDRV_BINDIR})
install(
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/xboxdrvctl
  DESTINATION ${XBOXDRV_BINDIR})
install(
  FILES doc/xboxdrv.1 doc/virtualkeyboard.1
  DESTINATION ${XBOXDRV_MANDIR}/man1)

# EOF #
