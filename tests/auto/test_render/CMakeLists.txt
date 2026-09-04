# SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

project(test.render)

find_package(Qt6 REQUIRED COMPONENTS Test)

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

link_directories(${MD_PDF_SKIA_BUILD_PATH})

set(NORMAL_FONT ${CMAKE_CURRENT_SOURCE_DIR}/../../fonts/DroidSerif/droid_serif.ttf)
set(ITALIC_FONT ${CMAKE_CURRENT_SOURCE_DIR}/../../fonts/DroidSerif/droid_serif_italic.ttf)
set(BOLD_FONT ${CMAKE_CURRENT_SOURCE_DIR}/../../fonts/DroidSerif/droid_serif_bold.ttf)
set(BOLD_ITALIC_FONT ${CMAKE_CURRENT_SOURCE_DIR}/../../fonts/DroidSerif/droid_serif_italic_bold.ttf)
set(MONO_FONT ${CMAKE_CURRENT_SOURCE_DIR}/../../fonts/SpaceMono/SpaceMono-Regular.ttf)
set(MONO_ITALIC_FONT ${CMAKE_CURRENT_SOURCE_DIR}/../../fonts/SpaceMono/SpaceMono-Italic.ttf)
set(MONO_BOLD_FONT ${CMAKE_CURRENT_SOURCE_DIR}/../../fonts/SpaceMono/SpaceMono-Bold.ttf)
set(MONO_BOLD_ITALIC_FONT ${CMAKE_CURRENT_SOURCE_DIR}/../../fonts/SpaceMono/SpaceMono-BoldItalic.ttf)
set(WORKING_FOLDER ${CMAKE_CURRENT_SOURCE_DIR})

configure_file(test_const.h.in test_const.h @ONLY)

add_library(md-pdf-lib-test-consts INTERFACE)

target_include_directories(md-pdf-lib-test-consts INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

set(BUILD_TEST_LIBRARY ON CACHE INTERNAL "" FORCE)
add_subdirectory(../../../3rdparty/md-pdf-lib "${CMAKE_CURRENT_BINARY_DIR}/md-pdf-lib-test")

add_executable(test.render ${SRC})

target_link_libraries(test.render
    md4qt::md4qt
    md-pdf-lib-test
    Qt6::Test)

add_test(NAME test.render
    COMMAND test.render${CMAKE_EXECUTABLE_SUFFIX}
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../../../bin)
