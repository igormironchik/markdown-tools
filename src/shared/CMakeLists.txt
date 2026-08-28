# SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

project(markdown-widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6Core REQUIRED)
find_package(Qt6Widgets REQUIRED)
find_package(KF${KF_MAJOR_VERSION} ${KF_MIN_VERSION} REQUIRED COMPONENTS
    SyntaxHighlighting WidgetsAddons
)
find_package(KF${KF_MAJOR_VERSION} ${KF_MIN_VERSION} COMPONENTS
    IconThemes ColorScheme Config
)

if(WIN32)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(SKIA_CFLAGS "/MD")
    else()
        set(SKIA_CFLAGS "/MDd")
    endif()
endif()

set(SRC license_dialog.h
	license_dialog.cpp
	license_dialog.ui
	utils.h
	utils.cpp
	folder_chooser.h
    folder_chooser.cpp
    plugins_page.cpp
    plugins_page.h
    plugins_page.ui
    switch.cpp
    switch.h
    color.h
    color.cpp
    fingergeometry.h
    fingergeometry.cpp
    syntax.h
    syntax.cpp
    dlg_filter_wheel.cpp
    dlg_filter_wheel.h
    emphasis.h
    emphasis.cpp
    emoji_parser.h
    emoji_parser.cpp
    emoji.h
    md.h)

qt6_add_resources(SRC qt.qrc)
qt6_add_resources(SRC icon.qrc)
qt6_add_resources(SRC tr.qrc)

install(FILES "pics/icon_16x16.png" DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/16x16/apps/" RENAME "markdown-tools.png")
install(FILES "pics/icon_24x24.png" DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/24x24/apps/" RENAME "markdown-tools.png")
install(FILES "pics/icon_32x32.png" DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/32x32/apps/" RENAME "markdown-tools.png")
install(FILES "pics/icon_48x48.png" DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/48x48/apps/" RENAME "markdown-tools.png")
install(FILES "pics/icon_64x64.png" DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/64x64/apps/" RENAME "markdown-tools.png")
install(FILES "pics/icon_128x128.png" DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/128x128/apps/" RENAME "markdown-tools.png")
install(FILES "pics/icon_256x256.png" DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/256x256/apps/" RENAME "markdown-tools.png")
    
include_directories(${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/Skia)

add_library(markdown-widgets STATIC ${SRC})

if(KF${KF_MAJOR_VERSION}IconThemes_FOUND AND KF${KF_MAJOR_VERSION}ColorScheme_FOUND
    AND KF${KF_MAJOR_VERSION}Config_FOUND)
    list(APPEND ADDITIONAL_TARGETS KF${KF_MAJOR_VERSION}::ConfigCore
        KF${KF_MAJOR_VERSION}::IconThemes KF${KF_MAJOR_VERSION}::ColorScheme)
    target_compile_definitions(markdown-widgets PRIVATE MD_BREEZE)
endif()

configure_file(version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h)
configure_file(version.number.in ${CMAKE_CURRENT_BINARY_DIR}/version.number)

target_link_libraries(markdown-widgets
    KF${KF_MAJOR_VERSION}::SyntaxHighlighting
    KF${KF_MAJOR_VERSION}::WidgetsAddons
    ${ADDITIONAL_TARGETS}
    Qt6::Widgets
    Qt6::Core
    skunicode_icu
    skunicode_core
    icu)

target_include_directories(markdown-widgets INTERFACE ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/markdown-widgets_autogen/include)

set_property(TARGET markdown-widgets PROPERTY CXX_STANDARD 17)

find_program(SYSTEM_GN gn)

if(SYSTEM_GN)
    set(GN_EXEC "${SYSTEM_GN}")
    set(FETCH_GN_COMMAND "")

    set(SKIA_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/Skia/bin")
    set(SKIA_GN_TARGET "${SKIA_BIN_DIR}/gn")

    if(NOT EXISTS "${SKIA_GN_TARGET}")
        message(STATUS "Creating symlink for system GN: ${SKIA_GN_TARGET} -> ${SYSTEM_GN}")
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E create_symlink "${SYSTEM_GN}" "${SKIA_GN_TARGET}"
            RESULT_VARIABLE LINK_RESULT
        )
        if(NOT LINK_RESULT EQUAL 0)
            message(FATAL_ERROR "Failed to create symlink for GN")
        endif()
    endif()
else()
    set(GN_EXEC "bin/gn")
    set(FETCH_GN_COMMAND COMMAND python3 bin/fetch-gn)
endif()

add_custom_command(
    TARGET markdown-widgets
    PRE_BUILD
    COMMAND ${FETCH_GN_COMMAND}
    COMMAND "${GN_EXEC}" gen "../../../builds/skia" "--args=is_official_build=true skia_use_system_expat=false skia_use_system_harfbuzz=false skia_use_system_icu=false skia_use_system_libjpeg_turbo=false skia_use_system_libpng=false skia_use_system_libwebp=false skia_use_system_zlib=false extra_cflags=[\"${SKIA_CFLAGS}\"]"
    COMMAND ninja -C "../../../builds/skia"
    COMMENT "Configuring and building Skia..."
    VERBATIM
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/Skia"
)
