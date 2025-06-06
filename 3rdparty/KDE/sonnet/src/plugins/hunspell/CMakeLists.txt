qt_add_plugin(sonnet_hunspell CLASS_NAME HunspellClient)

target_sources(sonnet_hunspell PRIVATE
    hunspellclient.cpp
    hunspelldict.cpp
)

ecm_qt_declare_logging_category(sonnet_hunspell
    HEADER hunspelldebug.h
    IDENTIFIER SONNET_HUNSPELL
    CATEGORY_NAME kf.sonnet.clients.hunspell
    OLD_CATEGORY_NAMES sonnet.plugins.hunspell
    DESCRIPTION "Sonnet HUnspell plugin"
    EXPORT SONNET
)

# see: https://phabricator.kde.org/R246:0a96acf251baa5c9dd042d093ab2bf8fcee10502
set(USE_OLD_HUNSPELL_API TRUE)
if (PKG_HUNSPELL_VERSION GREATER "1.5.0")
    set(USE_OLD_HUNSPELL_API FALSE) # new API introduced in v1.5.1 (cf. https://github.com/hunspell/hunspell/commit/8006703dafeebce19f2144c5cf180812eb99693a)
endif()
message(STATUS "Using old hunspell API: ${USE_OLD_HUNSPELL_API}")

configure_file(config-hunspell.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-hunspell.h)

target_include_directories(sonnet_hunspell SYSTEM PUBLIC ${HUNSPELL_INCLUDE_DIRS})
target_link_libraries(sonnet_hunspell PRIVATE KF6::SonnetCore ${HUNSPELL_LIBRARIES})

target_compile_definitions(sonnet_hunspell PRIVATE DEFINITIONS SONNET_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")

install(TARGETS sonnet_hunspell  DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/sonnet/)

set(SONNET_BACKEND_FOUND TRUE PARENT_SCOPE)
