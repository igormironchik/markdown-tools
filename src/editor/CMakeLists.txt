# SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

set( CMAKE_AUTOMOC ON )
set( CMAKE_AUTORCC ON )
set( CMAKE_AUTOUIC ON )

find_package( Qt6Core REQUIRED )
find_package( Qt6Widgets REQUIRED )
find_package( Qt6WebEngineWidgets REQUIRED )

set( LIB_SRC
	editor.hpp
	editor.cpp
	mainwindow.hpp
	mainwindow.cpp
	webview.hpp
	webview.cpp
	previewpage.hpp
	previewpage.cpp
	htmldocument.hpp
	htmldocument.cpp
	find.hpp
	find.cpp
	find.ui
	findweb.hpp
	findweb.cpp
	findweb.ui
	gotoline.hpp
	gotoline.cpp
	gotoline.ui
	fontdlg.hpp
	fontdlg.cpp
	fontdlg.ui
	colorsdlg.hpp
	colorsdlg.cpp
	colorsdlg.ui
	syntaxvisitor.cpp
	syntaxvisitor.hpp
	closebutton.hpp
	closebutton.cpp
	toc.hpp
	toc.cpp
	wordwrapdelegate.hpp
	wordwrapdelegate.cpp
	settings.ui
	font.ui
	colors.ui
	colors.hpp
	colors.cpp
	font.hpp
	font.cpp
	settings.hpp
	settings.cpp )

set( SRC main.cpp )

qt6_add_resources( LIB_SRC resources.qrc )

if( WIN32 )
	list( APPEND SRC md-editor.rc )
endif()

configure_file( version.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/version.hpp )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}
	${md4qt_INCLUDE_DIRECTORIES}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/widgets/include
	${CMAKE_CURRENT_SOURCE_DIR}/../shared )

add_library( md-editor-lib STATIC ${LIB_SRC} )

target_link_libraries( md-editor-lib markdown-widgets Qt6::WebEngineWidgets Qt6::Widgets Qt6::Core )

add_executable( md-editor WIN32 ${SRC} )

install( TARGETS md-editor )

target_link_libraries( md-editor md-editor-lib markdown-widgets Qt6::WebEngineWidgets Qt6::Widgets Qt6::Core )
