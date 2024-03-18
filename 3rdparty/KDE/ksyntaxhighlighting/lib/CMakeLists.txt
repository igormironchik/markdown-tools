
set( CMAKE_AUTOMOC ON )
set( CMAKE_AUTORCC ON )
set( CMAKE_AUTOUIC ON )

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(KF6SyntaxHighlighting)

target_sources(KF6SyntaxHighlighting PRIVATE
    abstracthighlighter.cpp
    context.cpp
    contextswitch.cpp
    definitiondownloader.cpp
    highlightingdata.cpp
    foldingregion.cpp
    format.cpp
    htmlhighlighter.cpp
    ansihighlighter.cpp
    keywordlist.cpp
    rule.cpp
    definition.cpp
    repository.cpp
    state.cpp
    syntaxhighlighter.cpp
    theme.cpp
    wildcardmatcher.cpp
    themedata.cpp
    worddelimiters.cpp
	${CMAKE_CURRENT_BINARY_DIR}/../data/syntax-data.qrc
	${CMAKE_CURRENT_SOURCE_DIR}/../data/themes/theme-data.qrc
)

ecm_generate_export_header(KF6SyntaxHighlighting
    BASE_NAME KSyntaxHighlighting
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT 0
)

ecm_qt_declare_logging_category(KF6SyntaxHighlighting
    HEADER ksyntaxhighlighting_logging.h
    IDENTIFIER KSyntaxHighlighting::Log
    CATEGORY_NAME kf.syntaxhighlighting
    OLD_CATEGORY_NAMES org.kde.ksyntaxhighlighting
    DESCRIPTION "Syntax Highlighting"
    EXPORT KSYNTAXHIGHLIGHTING
)

target_link_libraries(KF6SyntaxHighlighting
    PUBLIC
        Qt6::Gui
    PRIVATE
        Qt6::Network
)

target_compile_definitions(KF6SyntaxHighlighting PRIVATE HAS_SYNTAX_RESOURCE)
