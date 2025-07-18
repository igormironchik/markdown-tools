# SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

project(markdown-widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6Core REQUIRED)
find_package(Qt6Widgets REQUIRED)

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
    fingergeometry.cpp)

qt6_add_resources(SRC qt.qrc)
qt6_add_resources(SRC icon.qrc)
    
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(markdown-widgets STATIC ${SRC})

target_link_libraries(markdown-widgets Qt6::Widgets Qt6::Core)

set_property(TARGET markdown-widgets PROPERTY CXX_STANDARD 17)

