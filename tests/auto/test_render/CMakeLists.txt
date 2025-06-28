# SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

project(test.render)

find_package(Qt6Test 6.5.0 REQUIRED)
find_package(Qt6Gui 6.5.0 REQUIRED)
find_package(Qt6Widgets 6.5.0 REQUIRED)
find_package(Qt6Network 6.5.0 REQUIRED)

add_definitions(-DPODOFO_SHARED)

set(CMAKE_AUTOMOC ON)

if(ENABLE_COVERAGE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif(ENABLE_COVERAGE)

set(SRC main.cpp
	../../../src/converter/renderer.cpp
	../../../src/converter/renderer.h
	../../../src/converter/podofo_paintdevice.cpp
	../../../src/converter/podofo_paintdevice.h)

add_definitions(-DMD_PDF_TESTING -DBUILD_QT)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../../..
	${CMAKE_CURRENT_SOURCE_DIR}/../../../3rdparty
	${CMAKE_CURRENT_SOURCE_DIR}/../../../3rdparty/podofo/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../3rdparty/MicroTeX/src
	${CMAKE_CURRENT_BINARY_DIR}/../../../3rdparty/podofo/src/podofo
    ${CMAKE_CURRENT_BINARY_DIR}/../../../3rdparty/podofo/src/podofo/podofo/auxiliary
	${md4qt_INCLUDE_DIRECTORIES}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../../../3rdparty/ksyntaxhighlighting/lib
    ${CMAKE_CURRENT_BINARY_DIR}/../../../3rdparty/ksyntaxhighlighting/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../src/shared)

link_directories(${CMAKE_CURRENT_BINARY_DIR}/../../../3rdparty/podofo/src/podofo)
link_directories(${CMAKE_CURRENT_BINARY_DIR}/../../../3rdparty/podofo/src)

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

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../lib)

qt6_add_resources(SRC ${CMAKE_CURRENT_SOURCE_DIR}/../../../src/converter/resources.qrc)
qt6_add_resources(SRC ../../../3rdparty/MicroTeX/latex.qrc)

add_executable(test.render ${SRC})

target_link_libraries(test.render syntax podofo_shared markdown-widgets LaTeX
	Qt6::Widgets Qt6::Gui Qt6::Network Qt6::Test Qt6::Core resvg)

add_test(NAME test.render
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/../../../bin/md-launcher${CMAKE_EXECUTABLE_SUFFIX} --exe test.render${CMAKE_EXECUTABLE_SUFFIX}
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../../../bin)
