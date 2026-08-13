# SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

project(test.render)

find_package(Qt6Test 6.5.0 REQUIRED)

set(CMAKE_AUTOMOC ON)

if(ENABLE_COVERAGE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif(ENABLE_COVERAGE)

add_definitions(-DMD_PDF_TESTING -DBUILD_QT)

set(SRC main.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../../..
	${CMAKE_CURRENT_SOURCE_DIR}/../../../3rdparty
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../3rdparty/MicroTeX/src
	${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../src/shared
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../3rdparty/Skia)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../lib
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../builds/skia")

add_executable(test.render ${SRC})

target_link_libraries(test.render
    md4qt::md4qt
    md-pdf-lib-test
    Qt6::Test)

add_test(NAME test.render
    COMMAND test.render${CMAKE_EXECUTABLE_SUFFIX}
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../../../bin)
