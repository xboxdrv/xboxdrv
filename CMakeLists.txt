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

project(xboxdrv LANGUAGES CXX)

# --- Dependencies ---

include(GNUInstallDirs)

find_package(PkgConfig REQUIRED)

pkg_check_modules(DEPS REQUIRED IMPORTED_TARGET
    dbus-glib-1
    glib-2.0
    gthread-2.0
    libusb-1.0
    libudev
)

find_package(X11 REQUIRED)
find_package(Python3 COMPONENTS Interpreter REQUIRED)

# --- Versioning ---

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION")
    file(READ "VERSION" PACKAGE_VERSION)
    string(STRIP ${PACKAGE_VERSION} PACKAGE_VERSION)
else()
    set(PACKAGE_VERSION "0.0.0-unknown")
endif()

# --- Code Generation ---

set(GEN_HDR_DIR ${CMAKE_CURRENT_BINARY_DIR}/src)
file(MAKE_DIRECTORY ${GEN_HDR_DIR})

# Helper definitions
set(DBUS_GLUE_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/dbus_glue.py)
set(BIN2H_SCRIPT     ${CMAKE_CURRENT_SOURCE_DIR}/bin2h.py)

# Generate xboxdrv_vfs.hpp
set(VFS_OUTPUT ${GEN_HDR_DIR}/xboxdrv_vfs.hpp)
set(VFS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/mouse.xboxdrv
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/xpad-wireless.xboxdrv
)

add_custom_command(
    OUTPUT ${VFS_OUTPUT}
    DEPENDS ${VFS_SOURCES} ${BIN2H_SCRIPT}
    COMMAND ${Python3_EXECUTABLE} ${BIN2H_SCRIPT}
            --output ${VFS_OUTPUT}
            --prefix ${CMAKE_CURRENT_SOURCE_DIR}
            --namespace=xboxdrv_vfs
            ${VFS_SOURCES}
    COMMENT "Generating VFS header"
)

# Generate xboxdrv_controller_glue.hpp
set(CTRL_GLUE_OUTPUT ${GEN_HDR_DIR}/xboxdrv_controller_glue.hpp)
set(CTRL_GLUE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/xboxdrv_controller.xml)

add_custom_command(
    OUTPUT ${CTRL_GLUE_OUTPUT}
    DEPENDS ${CTRL_GLUE_SOURCE} ${DBUS_GLUE_SCRIPT}
    COMMAND ${Python3_EXECUTABLE} ${DBUS_GLUE_SCRIPT}
            -o ${CTRL_GLUE_OUTPUT} ${CTRL_GLUE_SOURCE}
            --dbus-prefix=xboxdrv_controller
    COMMENT "Generating Controller Glue"
)

# Generate xboxdrv_daemon_glue.hpp
set(DAEMON_GLUE_OUTPUT ${GEN_HDR_DIR}/xboxdrv_daemon_glue.hpp)
set(DAEMON_GLUE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/xboxdrv_daemon.xml)

add_custom_command(
    OUTPUT ${DAEMON_GLUE_OUTPUT}
    DEPENDS ${DAEMON_GLUE_SOURCE} ${DBUS_GLUE_SCRIPT}
    COMMAND ${Python3_EXECUTABLE} ${DBUS_GLUE_SCRIPT}
            -o ${DAEMON_GLUE_OUTPUT} ${DAEMON_GLUE_SOURCE}
            --dbus-prefix=xboxdrv_daemon
    COMMENT "Generating Daemon Glue"
)

# --- Library ---

file(GLOB LIB_SOURCES CONFIGURE_DEPENDS
    src/*.cpp
    src/axisfilter/*.cpp
    src/buttonfilter/*.cpp
    src/axisevent/*.cpp
    src/buttonevent/*.cpp
    src/modifier/*.cpp
)

# Add generated files to the source list so CMake knows to build them
list(APPEND LIB_SOURCES
    ${VFS_OUTPUT}
    ${CTRL_GLUE_OUTPUT}
    ${DAEMON_GLUE_OUTPUT}
)

add_library(libxboxdrv STATIC ${LIB_SOURCES})

# Target-specific Include Directories
target_include_directories(libxboxdrv PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${GEN_HDR_DIR}>
)

# Dependencies
target_link_libraries(libxboxdrv PUBLIC
    PkgConfig::DEPS
    X11::X11
)

# Compiler Flags & Definitions
target_compile_features(libxboxdrv PUBLIC cxx_std_20)
target_compile_definitions(libxboxdrv PUBLIC PACKAGE_VERSION="${PACKAGE_VERSION}")

# Define compiler warnings/flags
set(CXX_WARNINGS
    -Wall -pedantic
    $<$<CONFIG:Debug>:-Wextra -Werror -Wnon-virtual-dtor -Weffc++ -Wold-style-cast -Wshadow -Wcast-qual -Winit-self -Wno-unused-parameter>
)
target_compile_options(libxboxdrv PRIVATE ${CXX_WARNINGS})


# --- Executable ---

add_executable(xboxdrv src/main/main.cpp)
target_link_libraries(xboxdrv PRIVATE libxboxdrv)

# --- Tests ---

include(CTest)
if(BUILD_TESTING)
    file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "test/*_test.cpp")

    foreach(TEST_FILE ${TEST_SOURCES})
        get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)

        add_executable(${TEST_NAME} ${TEST_FILE})
        target_link_libraries(${TEST_NAME} PRIVATE libxboxdrv)

        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    endforeach()
endif()

# --- Installation ---

install(TARGETS xboxdrv
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES xboxdrvctl
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    PERMISSIONS OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
)

install(FILES doc/xboxdrv.1
    DESTINATION ${CMAKE_INSTALL_MANDIR}/man1
  )

# EOF #
