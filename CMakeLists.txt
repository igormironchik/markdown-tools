# SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.19)

project(markdown-tools)

option(BUILD_MD_TESTS "Build tests? Default OFF." OFF)

set(ENV{PKG_CONFIG_PATH} "${CMAKE_CURRENT_BINARY_DIR}")

list(PREPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_BINARY_DIR})
list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})

set(KF_MIN_VERSION "6")
set(KF_MAJOR_VERSION "6")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH "${ECM_MODULE_PATH}")

include(ECMEnableSanitizers)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)

if(NOT MARKDOWN_TOOLS_VERSION_MINOR)
	execute_process(COMMAND git rev-parse --short HEAD
		OUTPUT_VARIABLE MARKDOWN_TOOLS_VERSION_MINOR
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

set(MD_TOOLS_VERSION_NUMBER "0.17.0")
set(HomeDir "@HomeDir@")
string(TIMESTAMP MD_CURRENT_DATE "%Y-%m-%d")

add_compile_definitions(QT_NO_KEYWORDS)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.xml.in ${CMAKE_CURRENT_SOURCE_DIR}/installer/config/config.xml @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/package.xml.in ${CMAKE_CURRENT_SOURCE_DIR}/installer/packages/mironchik.igor.markdown/meta/package.xml @ONLY)

set(MD_TOOLS_VERSION "${MD_TOOLS_VERSION_NUMBER} build ${MARKDOWN_TOOLS_VERSION_MINOR}")
set(MD_TOOLS_COMMIT "${MARKDOWN_TOOLS_VERSION_MINOR}")

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)

if(UNIX AND NOT APPLE)
	list(PREPEND CMAKE_INSTALL_RPATH "\$ORIGIN/../lib")
else()
	list(PREPEND CMAKE_INSTALL_RPATH "@executable_path/../lib")
	list(PREPEND CMAKE_INSTALL_RPATH "@loader_path/../lib")
endif()

set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_MACOSX_RPATH TRUE)

link_directories(${CMAKE_BINARY_DIR}/lib)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release"
		CACHE STRING "Choose the type of build."
		FORCE)
endif(NOT CMAKE_BUILD_TYPE)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED _DISABLE_INSTALLATION)
  set(_DISABLE_INSTALLATION FALSE)

	macro(install)
		if(NOT _DISABLE_INSTALLATION)
			_install(${ARGN})
		endif()
	endmacro()
endif()

install(FILES "markdown-tools.desktop" DESTINATION "${CMAKE_INSTALL_PREFIX}/share/applications/")

set(_DISABLE_INSTALLATION TRUE)

add_subdirectory(3rdparty/MicroTeX)

set(BUILD_MD4QT_TESTS OFF CACHE INTERNAL "" FORCE)
add_subdirectory(3rdparty/KDE/md4qt)

include_directories(${md4qt_INCLUDE_DIRECTORIES}/../..)

set(_DISABLE_INSTALLATION FALSE)

string(REPLACE "/DNDEBUG" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

add_subdirectory(3rdparty/github-release)

add_subdirectory(src)

if(BUILD_MD_TESTS)
	enable_testing()

	add_subdirectory(tests)
endif()
