
project( markdown-widgets )

set( CMAKE_AUTOMOC ON )
set( CMAKE_AUTORCC ON )
set( CMAKE_AUTOUIC ON )

find_package( Qt6Core REQUIRED )
find_package( Qt6Widgets REQUIRED )

set( SRC license_dialog.hpp
	license_dialog.cpp
	license_dialog.ui
	color_widget.cpp
	color_widget.hpp )
    
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

add_library( markdown-widgets STATIC ${SRC} )

target_link_libraries( markdown-widgets Qt6::Widgets Qt6::Core )

set_property( TARGET markdown-widgets PROPERTY CXX_STANDARD 17 )

