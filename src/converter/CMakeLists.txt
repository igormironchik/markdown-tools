# SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

project(md-pdf-gui)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(KF${KF_MAJOR_VERSION} ${KF_MIN_VERSION} REQUIRED COMPONENTS
    SyntaxHighlighting
)

find_package(Qt6Widgets 6.5.0 REQUIRED)
find_package(Qt6Network 6.5.0 REQUIRED)
find_package(Qt6Svg 6.5.0 REQUIRED)
find_package(OpenSSL REQUIRED)

add_definitions(-DPODOFO_SHARED)

if(ENABLE_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif(ENABLE_COVERAGE)

set(GUI_SRC main.cpp
	main_window.cpp
	main_window.h
	main_window.ui
	podofo_paintdevice.h
	podofo_paintdevice.cpp
	renderer.h
	renderer.cpp
	progress.h
	progress.cpp
	progress.ui
    const.h
    settings.h
    settings.cpp
    settings.ui)
	
qt6_add_resources(GUI_SRC resources.qrc)
qt6_add_resources(GUI_SRC ../../3rdparty/MicroTeX/latex.qrc)

if(WIN32)
    list(APPEND GUI_SRC md-pdf.rc)
endif()

set(LIB_SYNTAX_SRC syntax.cpp
    syntax.h)

message(${CMAKE_CURRENT_BINARY_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/podofo/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/MicroTeX/src
	${CMAKE_CURRENT_BINARY_DIR}/../../3rdparty/podofo/src/podofo
    ${CMAKE_CURRENT_BINARY_DIR}/../../3rdparty/podofo/src/podofo/podofo/auxiliary
	${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared)

link_directories(${CMAKE_CURRENT_BINARY_DIR}/../../3rdparty/podofo/src/podofo
    ${CMAKE_CURRENT_BINARY_DIR}/../../3rdparty/podofo/src)

configure_file(version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h)

add_definitions(-DBUILD_QT)

add_library(syntax STATIC ${LIB_SYNTAX_SRC})

target_link_libraries(syntax KF${KF_MAJOR_VERSION}::SyntaxHighlighting Qt6::Gui Qt6::Network Qt6::Core)

add_executable(md-pdf-gui WIN32 ${GUI_SRC})

add_executable(md-launcher launcher.cpp)

target_link_libraries(md-launcher Qt6::Core OpenSSL::SSL)

install(TARGETS md-pdf-gui)
install(TARGETS md-launcher)

target_link_libraries(md-pdf-gui syntax markdown-widgets
    KF${KF_MAJOR_VERSION}::SyntaxHighlighting LaTeX
	Qt6::Widgets Qt6::Network Qt6::Core
    podofo::podofo resvg)
