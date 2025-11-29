# Copyright (C) 2025 Ingo Ruhnke <grumbel@gmail.com>
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

cmake_minimum_required(VERSION 3.20)

project(xboxdrv CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Use GNUInstallDirs to set standard installation directories
include(GNUInstallDirs)

# The PREFIX/BINDIR/MANDIR variables are now managed by GNUInstallDirs
# The lines below are removed/commented out as they are now set by the module:
# set(PREFIX /usr CACHE STRING "Installation prefix")
# set(BINDIR ${PREFIX}/bin)
# set(MANDIR ${PREFIX}/share/man)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/src)

find_package(Python REQUIRED)
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

file(READ "VERSION" PACKAGE_VERSION)
string(STRIP ${PACKAGE_VERSION} PACKAGE_VERSION)

set(BASE_CXX_FLAGS "-g;-O3;-Wall;-pedantic")

if (BUILD STREQUAL "development")
    set(DEV_FLAGS "-O3 -g3 -pedantic -Wall -Wextra -Werror -Wnon-virtual-dtor -Weffc++ -Wold-style-cast -Wshadow -Wcast-qual -Winit-self -Wno-unused-parameter")
    set(CMAKE_CXX_FLAGS "${DEV_FLAGS}" CACHE STRING "C++ Compiler Flags for Development")
else()
    set(CMAKE_CXX_FLAGS "${BASE_CXX_FLAGS}" CACHE STRING "Default C++ Compiler Flags")
endif()

include_directories(src)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/src/)

find_package(PkgConfig REQUIRED)
pkg_check_modules(DEPS REQUIRED dbus-glib-1 glib-2.0 gthread-2.0 libusb-1.0 libudev)

list(TRANSFORM DEPS_CFLAGS REPLACE "-I" "-isystem")

include_directories(SYSTEM ${DEPS_INCLUDE_DIRS})
link_directories(${DEPS_LIBRARY_DIRS})

find_package(X11 REQUIRED)

find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(PYTHON_EXECUTABLE ${Python3_EXECUTABLE})

file(GLOB_RECURSE LIB_SOURCES
    src/*.cpp
    src/axisfilter/*.cpp
    src/buttonfilter/*.cpp
    src/axisevent/*.cpp
    src/buttonevent/*.cpp
    src/modifier/*.cpp
)

list(APPEND LIB_SOURCES
    src/xboxdrv_vfs.hpp
    src/xboxdrv_daemon_glue.hpp
    src/xboxdrv_controller_glue.hpp
)

# Static Library (renamed to libxboxdrv)
# By listing the generated headers in the source list, CMake automatically
# ensures the custom commands run before the compilation begins.
add_library(libxboxdrv STATIC ${LIB_SOURCES})
target_link_libraries(libxboxdrv ${DEPS_LIBRARIES} ${X11_LIBRARIES})
target_compile_definitions(libxboxdrv PUBLIC -DPACKAGE_VERSION="${PROJECT_VERSION}")

# REMOVED: Custom targets and add_dependencies are no longer needed.
# Executable
add_executable(xboxdrv src/main/main.cpp)
target_link_libraries(xboxdrv libxboxdrv)

file(GLOB TEST_SOURCES "test/*_test.cpp")
add_custom_target(tests)

foreach(TEST_FILE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_FILE})
    target_link_libraries(${TEST_NAME} libxboxdrv)
endforeach()

# --- Installation ---

# Install the compiled executable
# Use CMAKE_INSTALL_BINDIR from GNUInstallDirs instead of BINDIR
install(TARGETS xboxdrv
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install the python script as a file with executable permissions
# Use CMAKE_INSTALL_BINDIR from GNUInstallDirs instead of BINDIR
install(FILES xboxdrvctl
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    PERMISSIONS OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
)

# Install man page
# Use CMAKE_INSTALL_MANDIR from GNUInstallDirs instead of MANDIR
install(FILES doc/xboxdrv.1
    DESTINATION ${CMAKE_INSTALL_MANDIR}/man1
  )

# EOF #
