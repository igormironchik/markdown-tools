# SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

project( test.render )

find_package( Qt6Core 6.5.0 REQUIRED )
find_package( Qt6Gui 6.5.0 REQUIRED )

if( ENABLE_COVERAGE )
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage" )
	set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage" )
endif( ENABLE_COVERAGE )

set( SRC main.cpp )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../../../src/editor
	${CMAKE_CURRENT_SOURCE_DIR}/../../../3rdparty
	${md4qt_INCLUDE_DIRECTORIES}
	${CMAKE_CURRENT_BINARY_DIR} )

set( TEST_DATA_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../../3rdparty/md4qt/tests/auto/test_parser/data/ )

configure_file( test_data_path.hpp.in test_data_path.hpp @ONLY )

add_executable( test.syntax_cache ${SRC} )

target_link_libraries( test.syntax_cache md-editor-lib Qt6::Gui Qt6::Core )

add_test( NAME test.syntax_cache COMMAND test.syntax_cache )
