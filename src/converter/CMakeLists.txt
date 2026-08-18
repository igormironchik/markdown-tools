# SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

project(md-pdf-gui)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6Widgets 6.5.0 REQUIRED)
find_package(Qt6Network 6.5.0 REQUIRED)
find_package(Qt6Svg 6.5.0 REQUIRED)
find_package(Qt6Test 6.5.0 REQUIRED)

find_package(KF${KF_MAJOR_VERSION} ${KF_MIN_VERSION} COMPONENTS
    IconThemes ColorScheme Config
)

find_package(ZLIB REQUIRED)

if(WIN32)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(SKIA_CFLAGS "/MD")
    else()
        set(SKIA_CFLAGS "/MDd")
    endif()
endif()

if(ENABLE_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif(ENABLE_COVERAGE)

set(LIB_SRC skia_paintdevice.h
    skia_paintdevice.cpp
    renderer.h
    renderer.cpp
    renderer_aux.h
    renderer_aux.cpp)

set(GUI_SRC main.cpp
	main_window.cpp
	main_window.h
	main_window.ui
	progress.h
	progress.cpp
	progress.ui
    const.h
    settings.h
    settings.cpp
    settings.ui)
	
qt6_add_resources(LIB_SRC resources.qrc)
qt6_add_resources(LIB_SRC ../../3rdparty/MicroTeX/latex.qrc)

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

set(NORMAL_FONT ${CMAKE_CURRENT_SOURCE_DIR}/../../tests/fonts/DroidSerif/droid_serif.ttf)
set(ITALIC_FONT ${CMAKE_CURRENT_SOURCE_DIR}/../../tests/fonts/DroidSerif/droid_serif_italic.ttf)
set(BOLD_FONT ${CMAKE_CURRENT_SOURCE_DIR}/../../tests/fonts/DroidSerif/droid_serif_bold.ttf)
set(BOLD_ITALIC_FONT ${CMAKE_CURRENT_SOURCE_DIR}/../../tests/fonts/DroidSerif/droid_serif_italic_bold.ttf)
set(MONO_FONT ${CMAKE_CURRENT_SOURCE_DIR}/../../tests/fonts/SpaceMono/SpaceMono-Regular.ttf)
set(MONO_ITALIC_FONT ${CMAKE_CURRENT_SOURCE_DIR}/../../tests/fonts/SpaceMono/SpaceMono-Italic.ttf)
set(MONO_BOLD_FONT ${CMAKE_CURRENT_SOURCE_DIR}/../../tests/fonts/SpaceMono/SpaceMono-Bold.ttf)
set(MONO_BOLD_ITALIC_FONT ${CMAKE_CURRENT_SOURCE_DIR}/../../tests/fonts/SpaceMono/SpaceMono-BoldItalic.ttf)
set(WORKING_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/../../tests/auto/test_render)

configure_file(test_const.h.in test_const.h @ONLY)

add_library(md-pdf-lib STATIC ${LIB_SRC})

add_library(md-pdf-lib-test STATIC ${LIB_SRC})

target_compile_definitions(md-pdf-lib-test PUBLIC MD_PDF_TESTING)

target_include_directories(md-pdf-lib-test PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

add_executable(md-pdf-gui WIN32 ${GUI_SRC})

if(KF${KF_MAJOR_VERSION}IconThemes_FOUND AND KF${KF_MAJOR_VERSION}ColorScheme_FOUND
    AND KF${KF_MAJOR_VERSION}Config_FOUND)
    list(APPEND ADDITIONAL_KDE_TARGETS KF${KF_MAJOR_VERSION}::ConfigCore
        KF${KF_MAJOR_VERSION}::IconThemes KF${KF_MAJOR_VERSION}::ColorScheme)
    target_compile_definitions(md-pdf-gui PRIVATE MD_BREEZE)
endif()

install(TARGETS md-pdf-gui)

target_link_libraries(md-pdf-lib
    md4qt::md4qt
    markdown-widgets
    LaTeX
    ${ADDITIONAL_KDE_TARGETS}
    Qt6::Network
    Qt6::Core
    bentleyottmann
    compression_utils_portable
    skresources
    skcms
    skottie
    skparagraph
    sksg
    svg
    wuffs
    skshaper
    skunicode_icu
    skunicode_core
    skia
    expat
    harfbuzz
    icu
    zlib
    ${PLATFORM_LIBS}
    ZLIB::ZLIB)

target_link_libraries(md-pdf-lib-test
    md4qt::md4qt
    markdown-widgets
    LaTeX
    ${ADDITIONAL_KDE_TARGETS}
    Qt6::Network
    Qt6::Core
    bentleyottmann
    compression_utils_portable
    skresources
    skcms
    skottie
    skparagraph
    sksg
    svg
    wuffs
    skshaper
    skunicode_icu
    skunicode_core
    skia
    expat
    harfbuzz
    icu
    zlib
    ${PLATFORM_LIBS}
    ZLIB::ZLIB
    Qt6::Test)

target_link_libraries(md-pdf-gui
    md4qt::md4qt
    md-pdf-lib)
