# SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
# SPDX-License-Identifier: MIT

if(DEFINED ENV{LEX})
  set(FLEX_EXECUTABLE $ENV{LEX})
  set(FLEX_FOUND TRUE)

  # This is part from original FindFLEX.cmake
  # Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
  # file Copyright.txt or https://cmake.org/licensing for details.
  #============================================================
  # FLEX_TARGET (public macro)
  #============================================================
  #
  macro(FLEX_TARGET Name Input Output)

    set(FLEX_TARGET_PARAM_OPTIONS)
    set(FLEX_TARGET_PARAM_ONE_VALUE_KEYWORDS
      COMPILE_FLAGS
      DEFINES_FILE
      )
    set(FLEX_TARGET_PARAM_MULTI_VALUE_KEYWORDS)

    cmake_parse_arguments(
      FLEX_TARGET_ARG
      "${FLEX_TARGET_PARAM_OPTIONS}"
      "${FLEX_TARGET_PARAM_ONE_VALUE_KEYWORDS}"
      "${FLEX_TARGET_MULTI_VALUE_KEYWORDS}"
      ${ARGN}
      )

    set(FLEX_TARGET_usage "FLEX_TARGET(<Name> <Input> <Output> [COMPILE_FLAGS <string>] [DEFINES_FILE <string>]")

    if(NOT "${FLEX_TARGET_ARG_UNPARSED_ARGUMENTS}" STREQUAL "")
      message(SEND_ERROR ${FLEX_TARGET_usage})
    else()

      cmake_policy(GET CMP0098 _flex_CMP0098
          PARENT_SCOPE # undocumented, do not use outside of CMake
        )
      set(_flex_INPUT "${Input}")
      if("x${_flex_CMP0098}x" STREQUAL "xNEWx")
        set(_flex_WORKING_DIR "${CMAKE_CURRENT_BINARY_DIR}")
        if(NOT IS_ABSOLUTE "${_flex_INPUT}")
          set(_flex_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/${_flex_INPUT}")
        endif()
      else()
        set(_flex_WORKING_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
      endif()
      unset(_flex_CMP0098)

      set(_flex_OUTPUT "${Output}")
      if(NOT IS_ABSOLUTE ${_flex_OUTPUT})
        set(_flex_OUTPUT "${_flex_WORKING_DIR}/${_flex_OUTPUT}")
      endif()
      set(_flex_TARGET_OUTPUTS "${_flex_OUTPUT}")

      set(_flex_EXE_OPTS "")
      if(NOT "${FLEX_TARGET_ARG_COMPILE_FLAGS}" STREQUAL "")
        set(_flex_EXE_OPTS "${FLEX_TARGET_ARG_COMPILE_FLAGS}")
        separate_arguments(_flex_EXE_OPTS)
      endif()

      set(_flex_OUTPUT_HEADER "")
      if(NOT "${FLEX_TARGET_ARG_DEFINES_FILE}" STREQUAL "")
        set(_flex_OUTPUT_HEADER "${FLEX_TARGET_ARG_DEFINES_FILE}")
        if(IS_ABSOLUTE "${_flex_OUTPUT_HEADER}")
          set(_flex_OUTPUT_HEADER_ABS "${_flex_OUTPUT_HEADER}")
        else()
          set(_flex_OUTPUT_HEADER_ABS "${_flex_WORKING_DIR}/${_flex_OUTPUT_HEADER}")
        endif()
        list(APPEND _flex_TARGET_OUTPUTS "${_flex_OUTPUT_HEADER_ABS}")
        list(APPEND _flex_EXE_OPTS --header-file=${_flex_OUTPUT_HEADER_ABS})
      endif()

      get_filename_component(_flex_EXE_NAME_WE "${FLEX_EXECUTABLE}" NAME_WE)
      add_custom_command(OUTPUT ${_flex_TARGET_OUTPUTS}
        COMMAND ${FLEX_EXECUTABLE} ${_flex_EXE_OPTS} -o${_flex_OUTPUT} ${_flex_INPUT}
        VERBATIM
        DEPENDS ${_flex_INPUT}
        COMMENT "[FLEX][${Name}] Building scanner with ${_flex_EXE_NAME_WE} ${FLEX_VERSION}"
        WORKING_DIRECTORY ${_flex_WORKING_DIR})

      set(FLEX_${Name}_DEFINED TRUE)
      set(FLEX_${Name}_OUTPUTS ${_flex_TARGET_OUTPUTS})
      set(FLEX_${Name}_INPUT ${_flex_INPUT})
      set(FLEX_${Name}_COMPILE_FLAGS ${_flex_EXE_OPTS})
      set(FLEX_${Name}_OUTPUT_HEADER ${_flex_OUTPUT_HEADER})

      unset(_flex_EXE_NAME_WE)
      unset(_flex_EXE_OPTS)
      unset(_flex_INPUT)
      unset(_flex_OUTPUT)
      unset(_flex_OUTPUT_HEADER)
      unset(_flex_OUTPUT_HEADER_ABS)
      unset(_flex_TARGET_OUTPUTS)
      unset(_flex_WORKING_DIR)

    endif()
  endmacro()
  #============================================================

  #============================================================
  # ADD_FLEX_BISON_DEPENDENCY (public macro)
  #============================================================
  #
  macro(ADD_FLEX_BISON_DEPENDENCY FlexTarget BisonTarget)

    if(NOT FLEX_${FlexTarget}_OUTPUTS)
      message(SEND_ERROR "Flex target `${FlexTarget}' does not exist.")
    endif()

    if(NOT BISON_${BisonTarget}_OUTPUT_HEADER)
      message(SEND_ERROR "Bison target `${BisonTarget}' does not exist.")
    endif()

    set_source_files_properties(${FLEX_${FlexTarget}_OUTPUTS}
      PROPERTIES OBJECT_DEPENDS ${BISON_${BisonTarget}_OUTPUT_HEADER})
  endmacro()
  #============================================================
endif()
