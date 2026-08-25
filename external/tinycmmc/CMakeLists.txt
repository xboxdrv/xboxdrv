# Copyright (C) 2022 Ingo Ruhnke <grumbel@gmail.com>
#
# This software is provided 'as-is', without any express or implied
# warranty.  In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.

cmake_minimum_required(VERSION 3.10)
project(tinycmmc VERSION 0.1.0)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(TINYCMMC_MODULE_INSTALL_DIR "${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_NAME}/cmake/modules/")
set(TINYCMMC_CONFIG_INSTALL_DIR "${CMAKE_INSTALL_FULL_LIBDIR}/cmake/${PROJECT_NAME}/")

install(DIRECTORY modules/
  DESTINATION ${TINYCMMC_MODULE_INSTALL_DIR})

configure_package_config_file(
  tinycmmcConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/tinycmmcConfig.cmake
  INSTALL_DESTINATION ${TINYCMMC_CONFIG_INSTALL_DIR}
  PATH_VARS TINYCMMC_MODULE_INSTALL_DIR
)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/tinycmmcConfigVersion.cmake
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY AnyNewerVersion
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/tinycmmcConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/tinycmmcConfigVersion.cmake
  DESTINATION ${TINYCMMC_CONFIG_INSTALL_DIR}
)

# EOF #
