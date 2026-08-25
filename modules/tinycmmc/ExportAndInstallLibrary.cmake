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

macro(tinycmmc_export_and_install_library _NAME)

  install(TARGETS "${_NAME}"
    EXPORT "${_NAME}"
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${_NAME}")

  add_library(${_NAME}::${_NAME} ALIAS ${_NAME})

  string(TOLOWER "${_NAME}" ${_NAME}_LOWERCASE)

  include(CMakePackageConfigHelpers)
  configure_package_config_file("${${_NAME}_LOWERCASE}-config.cmake.in" "${${_NAME}_LOWERCASE}-config.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/${_NAME}")
  write_basic_package_version_file("${${_NAME}_LOWERCASE}-config-version.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMinorVersion)
  export(EXPORT "${_NAME}"
    NAMESPACE "${_NAME}::"
    FILE "${${_NAME}_LOWERCASE}-targets.cmake")
  install(EXPORT "${_NAME}"
    FILE "${${_NAME}_LOWERCASE}-targets.cmake"
    NAMESPACE "${_NAME}::"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${_NAME}")
  install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${${_NAME}_LOWERCASE}-config-version.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${${_NAME}_LOWERCASE}-config.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${_NAME}")

endmacro()

# EOF #
