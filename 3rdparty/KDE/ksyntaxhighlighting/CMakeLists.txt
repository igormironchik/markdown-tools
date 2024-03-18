
cmake_minimum_required( VERSION 3.19 )

set(KF_VERSION "5.240.0")
project(KSyntaxHighlighting VERSION ${KF_VERSION})

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package( Qt6 REQUIRED COMPONENTS Gui Network )

include(ECMSetupVersion)
include(ECMGenerateExportHeader)
include(ECMQtDeclareLoggingCategory)

set(ksyntaxhighlighting_version_header "${CMAKE_CURRENT_BINARY_DIR}/lib/ksyntaxhighlighting_version.h")
ecm_setup_version(PROJECT
    VARIABLE_PREFIX KSYNTAXHIGHLIGHTING
    VERSION_HEADER "${ksyntaxhighlighting_version_header}"
    SOVERSION 6
)

set(CMAKE_AUTORCC ON)
set(QRC_SYNTAX ON)

add_compile_definitions(QT_USE_QSTRINGBUILDER)

add_subdirectory( indexer )
add_subdirectory( data )
add_subdirectory( lib )
