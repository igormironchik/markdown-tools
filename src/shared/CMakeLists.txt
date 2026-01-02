# SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

project(markdown-widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6Core REQUIRED)
find_package(Qt6Widgets REQUIRED)
find_package(KF${KF_MAJOR_VERSION} ${KF_MIN_VERSION} REQUIRED COMPONENTS
    SyntaxHighlighting
)

set(SRC license_dialog.h
	license_dialog.cpp
	license_dialog.ui
	color_widget.cpp
	color_widget.h
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
    emphasis.cpp)

qt6_add_resources(SRC qt.qrc)
qt6_add_resources(SRC icon.qrc)
qt6_add_resources(SRC tr.qrc)

install(FILES "icon/icon_16x16.png" DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/16x16/apps/" RENAME "markdown-tools.png")
install(FILES "icon/icon_24x24.png" DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/24x24/apps/" RENAME "markdown-tools.png")
install(FILES "icon/icon_32x32.png" DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/32x32/apps/" RENAME "markdown-tools.png")
install(FILES "icon/icon_48x48.png" DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/48x48/apps/" RENAME "markdown-tools.png")
install(FILES "icon/icon_64x64.png" DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/64x64/apps/" RENAME "markdown-tools.png")
install(FILES "icon/icon_128x128.png" DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/128x128/apps/" RENAME "markdown-tools.png")
install(FILES "icon/icon_256x256.png" DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/256x256/apps/" RENAME "markdown-tools.png")
    
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(markdown-widgets STATIC ${SRC})

target_link_libraries(markdown-widgets KF${KF_MAJOR_VERSION}::SyntaxHighlighting Qt6::Widgets Qt6::Core)

set_property(TARGET markdown-widgets PROPERTY CXX_STANDARD 17)

