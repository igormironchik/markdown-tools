# SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6Core REQUIRED)
find_package(Qt6Widgets REQUIRED)
find_package(Qt6WebEngineWidgets REQUIRED)
find_package(KF${KF_MAJOR_VERSION} ${KF_MIN_VERSION} REQUIRED COMPONENTS
    Sonnet SyntaxHighlighting
)

set(LIB_SRC
	editor.h
	editor.cpp
	mainwindow.h
	mainwindow.cpp
	webview.h
	webview.cpp
	previewpage.h
	previewpage.cpp
	htmldocument.h
	htmldocument.cpp
	find.h
	find.cpp
	find.ui
	findweb.h
	findweb.cpp
	findweb.ui
	gotoline.h
	gotoline.cpp
	gotoline.ui
	fontdlg.h
	fontdlg.cpp
	fontdlg.ui
	colorsdlg.h
	colorsdlg.cpp
	colorsdlg.ui
	syntaxvisitor.cpp
	syntaxvisitor.h
	closebutton.h
	closebutton.cpp
	toc.h
	toc.cpp
	wordwrapdelegate.h
	wordwrapdelegate.cpp
	settings.ui
	font.ui
	colors.ui
	colors.h
	colors.cpp
	font.h
	font.cpp
	settings.h
    settings.cpp
    sonnetconfig.h
    sonnetconfig.cpp
    speller.h
    speller.cpp)

set(SRC main.cpp)

qt6_add_resources(LIB_SRC resources.qrc)

if(WIN32)
    list(APPEND SRC md-editor.rc)
endif()

configure_file(version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/widgets/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared)

add_library(md-editor-lib STATIC ${LIB_SRC})

target_link_libraries(md-editor-lib md4qt::md4qt markdown-widgets
    KF${KF_MAJOR_VERSION}::SyntaxHighlighting
    KF${KF_MAJOR_VERSION}::SonnetUi KF${KF_MAJOR_VERSION}::SonnetCore
    Qt6::WebEngineWidgets Qt6::Widgets Qt6::Core)

add_executable(md-editor WIN32 ${SRC})

install(TARGETS md-editor)

target_link_libraries(md-editor md-editor-lib markdown-widgets KF${KF_MAJOR_VERSION}::SonnetUi KF${KF_MAJOR_VERSION}::SonnetCore
    Qt6::WebEngineWidgets Qt6::Widgets Qt6::Core)
