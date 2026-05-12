add_library(KF6Crash)
add_library(KF6::Crash ALIAS KF6Crash)

set_target_properties(KF6Crash PROPERTIES
    VERSION     ${KCRASH_VERSION}
    SOVERSION   ${KCRASH_SOVERSION}
    EXPORT_NAME Crash
)

target_sources(KF6Crash PRIVATE
    coreconfig.cpp
    coreconfig_p.h
    kcrash.cpp
    kcrash.h
    metadata.cpp
    metadata_p.h
    exception.cpp
)

kde_source_files_enable_exceptions(exception.cpp)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config-kcrash.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config-kcrash.h
)

ecm_qt_declare_logging_category(
    KF6Crash
    HEADER kcrash_debug.h
    IDENTIFIER LOG_KCRASH
    CATEGORY_NAME kf.crash
    OLD_CATEGORY_NAMES org.kde.kcrash
    DESCRIPTION "KCrash"
    EXPORT KCRASH
)

ecm_generate_export_header(KF6Crash
    BASE_NAME KCrash
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
)

target_include_directories(KF6Crash INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KCrash>")

target_link_libraries(KF6Crash PUBLIC Qt6::Core)
target_link_libraries(KF6Crash PRIVATE Qt6::Gui KF6::CoreAddons)

if (HAIKU)
       target_link_libraries(KF6Crash PRIVATE network)
endif ()

# FIXME: It is needed to work around undefined reference error on FreeBSD
# caused by --no-undefined because the `environ' variable does not exist
# in libc.so.7 -- it is in crt1.o, and so not available to shared
# libraries. We just drop the normal no-undefined flag for library building
# here, on the assumption that Linux CI will catch any *actual* undefineds.
if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
    string(REPLACE "-Wl,--no-undefined" "" CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
endif()

if(WITH_X11)
    target_link_libraries(KF6Crash PRIVATE X11::X11)
endif()

ecm_generate_headers(KCrash_HEADERS
  HEADER_NAMES
  KCrash

  REQUIRED_HEADERS KCrash_HEADERS
)

install(TARGETS KF6Crash EXPORT KF6CrashTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kcrash_export.h
  ${KCrash_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KCrash COMPONENT Devel
)

ecm_qt_install_logging_categories(
    EXPORT KCRASH
    FILE kcrash.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

ecm_generate_qdoc(KF6Crash kcrash.qdocconf)
