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

include(FindPkgConfig)

pkg_search_module(DBUSGLIB REQUIRED dbus-glib-1)
pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_search_module(GTHREAD REQUIRED gthread-2.0)
pkg_search_module(USB REQUIRED libusb-1.0)
pkg_search_module(UDEV REQUIRED libudev)
pkg_search_module(X11 REQUIRED x11)

file(GLOB XBOXDRV_SOURCES_CXX RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  src/*.cpp
  src/axisevent/*.cpp
  src/axisfilter/*.cpp
  src/buttonevent/*.cpp
  src/buttonfilter/*.cpp
  src/controller/*.cpp
  src/modifier/*.cpp
  src/symbols/*.cpp)

include_directories(src/)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/src/)

add_custom_target(
  xboxdrv_vfs
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/bin2h.py -o ${CMAKE_CURRENT_BINARY_DIR}/src/xboxdrv_vfs.hpp --namespace=xboxdrv_vfs ${CMAKE_CURRENT_SOURCE_DIR}/examples/mouse.xboxdrv ${CMAKE_CURRENT_SOURCE_DIR}/examples/xpad-wireless.xboxdrv
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/examples/mouse.xboxdrv ${CMAKE_CURRENT_SOURCE_DIR}/examples/xpad-wireless.xboxdrv)

add_custom_target(
  xboxdrv_controller_glue
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/dbus_glue.py -o ${CMAKE_CURRENT_BINARY_DIR}/src/xboxdrv_controller_glue.hpp ${CMAKE_CURRENT_SOURCE_DIR}/src/xboxdrv_controller.xml --dbus-prefix=xboxdrv_controller
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/xboxdrv_controller.xml)

add_custom_target(
  xboxdrv_daemon_glue
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/dbus_glue.py -o ${CMAKE_CURRENT_BINARY_DIR}/src/xboxdrv_daemon_glue.hpp ${CMAKE_CURRENT_SOURCE_DIR}/src/xboxdrv_daemon.xml --dbus-prefix=xboxdrv_daemon
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/xboxdrv_daemon.xml)

add_library(libxboxdrv STATIC ${XBOXDRV_SOURCES_CXX})
set_target_properties(libxboxdrv PROPERTIES OUTPUT_NAME xboxdrv)

add_dependencies(libxboxdrv xboxdrv_controller_glue xboxdrv_daemon_glue xboxdrv_vfs)

file(STRINGS VERSION XBOXDRV_VERSION)
target_compile_definitions(libxboxdrv PUBLIC -DPACKAGE_VERSION="${XBOXDRV_VERSION}")

target_include_directories(libxboxdrv SYSTEM PUBLIC
  ${DBUSGLIB_INCLUDE_DIRS}
  ${GLIB_INCLUDE_DIRS}
  ${GTHREAD_INCLUDE_DIRS}
  ${USB_INCLUDE_DIRS}
  ${UDEV_INCLUDE_DIRS}
  ${X11_INCLUDE_DIRS})

add_executable(xboxdrv src/main/main.cpp)
target_link_libraries(xboxdrv libxboxdrv)
target_link_libraries(xboxdrv
  ${DBUSGLIB_LIBRARIES}
  ${GLIB_LIBRARIES}
  ${GTHREAD_LIBRARIES}
  ${USB_LIBRARIES}
  ${UDEV_LIBRARIES}
  ${X11_LIBRARIES})

# EOF #
