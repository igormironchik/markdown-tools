# SPDX-FileCopyrightText: 2025 Azhar Momin <azhar.momin@kdemail.net>
# SPDX-License-Identifier: BSD-2-Clause

if(DEFINED ENV{LIB_FUZZING_ENGINE})
    set(fuzzing_engine $ENV{LIB_FUZZING_ENGINE})
else()
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(fuzzing_engine -fsanitize=fuzzer)
    else()
        message(FATAL_ERROR "Fuzzing engine not supported")
    endif()
endif()


add_executable(kconfig_fuzzer kconfig_fuzzer.cc)
target_link_libraries(kconfig_fuzzer PRIVATE KF6::ConfigCore ${fuzzing_engine})

set_target_properties(kconfig_fuzzer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/fuzzers
)
