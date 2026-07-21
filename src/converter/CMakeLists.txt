# SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

project(md-pdf-gui)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)

find_package(Qt6Widgets 6.5.0 REQUIRED)
find_package(Qt6Network 6.5.0 REQUIRED)
find_package(Qt6Svg 6.5.0 REQUIRED)
find_package(KF${KF_MAJOR_VERSION} ${KF_MIN_VERSION} COMPONENTS
    IconThemes ColorScheme Config
)
find_package(ZLIB REQUIRED)
find_package(Fontconfig REQUIRED)

get_target_property(FONTCONFIG_INCLUDE_DIR Fontconfig::Fontconfig INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(FONTCONFIG_LIBRARY Fontconfig::Fontconfig IMPORTED_LOCATION)
get_filename_component(FONTCONFIG_LIB_DIR "${FONTCONFIG_LIBRARY}" DIRECTORY)

set(SKIA_CFLAGS "-I${FONTCONFIG_INCLUDE_DIR}")
set(SKIA_LDFLAGS "-L${FONTCONFIG_LIB_DIR} -lfontconfig")

if(ENABLE_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif(ENABLE_COVERAGE)

set(GUI_SRC main.cpp
	main_window.cpp
	main_window.h
	main_window.ui
	skia_paintdevice.h
	skia_paintdevice.cpp
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

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/MicroTeX/src
	${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared
    ${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/Skia)

add_definitions(-DBUILD_QT)

link_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../../builds/skia")

add_executable(md-pdf-gui WIN32 ${GUI_SRC})

if(KF${KF_MAJOR_VERSION}IconThemes_FOUND AND KF${KF_MAJOR_VERSION}ColorScheme_FOUND
    AND KF${KF_MAJOR_VERSION}Config_FOUND)
    list(APPEND ADDITIONAL_KDE_TARGETS KF${KF_MAJOR_VERSION}::ConfigCore
        KF${KF_MAJOR_VERSION}::IconThemes KF${KF_MAJOR_VERSION}::ColorScheme)
    target_compile_definitions(md-pdf-gui PRIVATE MD_BREEZE)
endif()

install(TARGETS md-pdf-gui)

if(WIN32)
    set(WIN_LIBS Dwmapi)
endif()

if(UNIX)
    set(UNIX_LIBS freetype fontconfig)
endif()

target_link_libraries(md-pdf-gui
    md4qt::md4qt
    markdown-widgets
    LaTeX
    ${ADDITIONAL_KDE_TARGETS}
    Qt6::Widgets
    Qt6::Network
    Qt6::Core
    bentleyottmann
    compression_utils_portable
    skresources
    dng_sdk
    piex
    skcms
    skottie
    skparagraph
    sksg
    svg
    webp
    wuffs
    skshaper
    skia
    skunicode_icu
    skunicode_core
    png
    expat
    harfbuzz
    icu
    jpeg
    zlib
    ${WIN_LIBS}
    ${UNIX_LIBS}
    ZLIB::ZLIB)
    
add_custom_command(
    TARGET md-pdf-gui
    PRE_BUILD
    COMMAND "bin/gn" gen "../../../builds/skia" "--args=is_official_build=true skia_use_system_expat=false skia_use_system_harfbuzz=false skia_use_system_icu=false skia_use_system_libjpeg_turbo=false skia_use_system_libpng=false skia_use_system_libwebp=false skia_use_system_zlib=false extra_cflags=[\"${SKIA_CFLAGS}\"] extra_ldflags=[\"${SKIA_LDFLAGS}\"]"
    COMMAND ninja -C "../../../builds/skia"
    COMMENT "Configuring and building Skia..."
    VERBATIM
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/Skia"
)
