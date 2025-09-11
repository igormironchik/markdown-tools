# SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.19)

project(markdown-tools)

option(BUILD_MDPDF_TESTS "Build tests? Default OFF." OFF)
option(BUILD_WITH_SYSTEM_LIBS "Build with system's libraries? Default OFF." OFF)

set(ENV{PKG_CONFIG_PATH} "${CMAKE_CURRENT_BINARY_DIR}")

list(PREPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_BINARY_DIR})
list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})
list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

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

execute_process(COMMAND git rev-parse --short HEAD
	OUTPUT_VARIABLE MARKDOWN_TOOLS_VERSION_MINOR
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	OUTPUT_STRIP_TRAILING_WHITESPACE)

set(MD_TOOLS_VERSION_NUMBER "0.15.15")
set(HomeDir "@HomeDir@")
string(TIMESTAMP MD_CURRENT_DATE "%Y-%m-%d")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.xml.in ${CMAKE_CURRENT_SOURCE_DIR}/installer/config/config.xml @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/package.xml.in ${CMAKE_CURRENT_SOURCE_DIR}/installer/packages/mironchik.igor.markdown/meta/package.xml @ONLY)

set(MD_TOOLS_VERSION "${MD_TOOLS_VERSION_NUMBER} build ${MARKDOWN_TOOLS_VERSION_MINOR}")

set(MDPDF_VERSION ${MD_TOOLS_VERSION})
set(MDEDITOR_VERSION ${MD_TOOLS_VERSION})

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

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED _DISABLE_INSTALLATION)
  set(_DISABLE_INSTALLATION FALSE)

	macro(install)
		if(NOT _DISABLE_INSTALLATION)
			_install(${ARGN})
		endif()
	endmacro()
endif()

set(_DISABLE_INSTALLATION TRUE)

if(NOT BUILD_WITH_SYSTEM_LIBS)
    set(PODOFO_BUILD_LIB_ONLY ON CACHE INTERNAL "" FORCE)
    add_subdirectory(3rdparty/podofo)
else()
    find_package(podofo REQUIRED)
    add_library(podofo_shared ALIAS podofo::podofo_shared)
endif()

add_subdirectory(3rdparty/MicroTeX)

set(BUILD_MD4QT_QT_TESTS OFF CACHE INTERNAL "" FORCE)
add_subdirectory(3rdparty/KDE/md4qt)

include_directories(${md4qt_INCLUDE_DIRECTORIES})

set(_DISABLE_INSTALLATION FALSE)

if (NOT BUILD_WITH_SYSTEM_LIBS)
    install(TARGETS podofo_shared)

    file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/resvg.version RESVG_VERSION)
    file(ARCHIVE_EXTRACT
        INPUT ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/resvg-${RESVG_VERSION}.tar.xz
        DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty)

    add_library(resvg UNKNOWN IMPORTED)
    add_custom_target(build_resvg ALL
        COMMAND cargo build --release
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/resvg-${RESVG_VERSION}/crates/c-api"
        BYPRODUCTS ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/resvg-${RESVG_VERSION}/target/release/${CMAKE_STATIC_LIBRARY_PREFIX}resvg${CMAKE_SHARED_LIBRARY_SUFFIX}${CMAKE_STATIC_LIBRARY_SUFFIX})

    if(UNIX AND NOT APPLE)
        add_custom_command(TARGET build_resvg POST_BUILD
            COMMAND patchelf --set-soname libresvg.so libresvg.so
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/resvg-${RESVG_VERSION}/target/release")
    endif()

    if(WIN32)
        set_target_properties(resvg PROPERTIES
            IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/resvg-${RESVG_VERSION}/target/release/${CMAKE_STATIC_LIBRARY_PREFIX}resvg${CMAKE_SHARED_LIBRARY_SUFFIX}${CMAKE_STATIC_LIBRARY_SUFFIX})

        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/resvg-${RESVG_VERSION}/target/release/${CMAKE_STATIC_LIBRARY_PREFIX}resvg${CMAKE_SHARED_LIBRARY_SUFFIX}
            TYPE BIN)
    else()
        set_target_properties(resvg PROPERTIES
            IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/resvg-${RESVG_VERSION}/target/release/${CMAKE_STATIC_LIBRARY_PREFIX}resvg${CMAKE_SHARED_LIBRARY_SUFFIX})

        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/resvg-${RESVG_VERSION}/target/release/${CMAKE_STATIC_LIBRARY_PREFIX}resvg${CMAKE_SHARED_LIBRARY_SUFFIX}
            TYPE LIB)
    endif()

    set_target_properties(resvg PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/resvg-${RESVG_VERSION}/crates/c-api)
    add_dependencies(resvg build_resvg)
else()
    find_package(resvg REQUIRED)
endif()

add_subdirectory(src)

if(BUILD_MDPDF_TESTS)
	enable_testing()

	add_subdirectory(tests)
endif()
