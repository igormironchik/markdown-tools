# SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

project(md-pdf-gui)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(KF${KF_MAJOR_VERSION} ${KF_MIN_VERSION} COMPONENTS
    IconThemes ColorScheme Config
)
find_package(KF${KF_MAJOR_VERSION} ${KF_MIN_VERSION} REQUIRED COMPONENTS
    WidgetsAddons
)
find_package(ZLIB REQUIRED)

if(ENABLE_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif(ENABLE_COVERAGE)

set(GUI_SRC main.cpp
	main_window.cpp
	main_window.h
	main_window.ui
	progress.h
	progress.cpp
	progress.ui
    settings.h
    settings.cpp
    settings.ui)
	
qt6_add_resources(LIB_SRC resources.qrc)

if(WIN32)
    list(APPEND GUI_SRC md-pdf.rc)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared)

link_directories(${MD_PDF_SKIA_BUILD_PATH})

add_executable(md-pdf-gui WIN32 ${GUI_SRC})

if(KF${KF_MAJOR_VERSION}IconThemes_FOUND AND KF${KF_MAJOR_VERSION}ColorScheme_FOUND
    AND KF${KF_MAJOR_VERSION}Config_FOUND)
    list(APPEND ADDITIONAL_KDE_TARGETS KF${KF_MAJOR_VERSION}::ConfigCore
        KF${KF_MAJOR_VERSION}::IconThemes KF${KF_MAJOR_VERSION}::ColorScheme)
    target_compile_definitions(md-pdf-gui PRIVATE MD_BREEZE)
endif()

install(TARGETS md-pdf-gui)

target_link_libraries(md-pdf-gui
    KF${KF_MAJOR_VERSION}::WidgetsAddons
    ${ADDITIONAL_KDE_TARGETS}
    markdown-widgets
    md-pdf-lib
    Qt6::Widgets)
