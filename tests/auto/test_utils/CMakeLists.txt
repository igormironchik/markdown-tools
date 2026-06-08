# SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

project(test.utils)

find_package(Qt6Test 6.5.0 REQUIRED)

set(CMAKE_AUTOMOC ON)

if(ENABLE_COVERAGE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif(ENABLE_COVERAGE)

set(SRC main.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../src/utils)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../lib)

add_executable(test.utils ${SRC})

target_link_libraries(test.utils md-utils Qt6::Test Qt6::Core)

add_test(NAME test.utils
	COMMAND test.utils${CMAKE_EXECUTABLE_SUFFIX}
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../../../bin)
