# Copyright (C) 2020 Ingo Ruhnke <grumbel@gmail.com>
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

function(install_with_wrapper)
  cmake_parse_arguments(ARGS "" "OPTION;DATADIR;DESTINATION" "TARGETS" ${ARGN})

  if(NOT ARGS_OPTION)
    set(ARGS_OPTION "--datadir")
  endif()

  if(NOT ARGS_DATADIR)
    set(ARGS_DATADIR "${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_NAME}")
  endif()

  if(NOT ARGS_LIBEXECDIR)
    set(ARGS_LIBEXECDIR "${CMAKE_INSTALL_FULL_LIBEXECDIR}/${PROJECT_NAME}")
  endif()

  if(NOT ARGS_DESTINATION)
    set(ARGS_DESTINATION "${CMAKE_INSTALL_FULL_BINDIR}")
  endif()

  foreach(TARGET ${ARGS_TARGETS})
    install(TARGETS ${TARGET} RUNTIME DESTINATION "${ARGS_LIBEXECDIR}")

    get_target_property(TARGET_EXENAME ${TARGET} OUTPUT_NAME)
    if(NOT TARGET_EXENAME)
      set(TARGET_EXENAME "${TARGET}")
    endif()

    file(WRITE "${CMAKE_BINARY_DIR}/${TARGET_EXENAME}-wrapper.sh"
      "#!/bin/sh\n"
      "exec \"${ARGS_LIBEXECDIR}/${TARGET_EXENAME}\""
      " ${ARGS_OPTION} \"${ARGS_DATADIR}\" \"$@\"\n")

    install(PROGRAMS
      "${CMAKE_BINARY_DIR}/${TARGET_EXENAME}-wrapper.sh"
      RENAME ${TARGET_EXENAME}
      DESTINATION "${ARGS_DESTINATION}")
  endforeach()
endfunction()

# EOF #
