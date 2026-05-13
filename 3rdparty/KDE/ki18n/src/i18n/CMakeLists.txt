if (NOT BUILD_SHARED_LIBS AND BUILD_WITH_QML)
    set(HAVE_STATIC_KTRANSCRIPT ON)
else()
    set(HAVE_STATIC_KTRANSCRIPT OFF)
endif()
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.h")

add_library(KF6I18n)
add_library(KF6::I18n ALIAS KF6I18n)

set_target_properties(KF6I18n PROPERTIES
    VERSION     ${KI18N_VERSION}
    SOVERSION   ${KI18N_SOVERSION}
    EXPORT_NAME I18n
)

target_sources(KF6I18n PRIVATE
    klocalizedstring.cpp
    klocalizedtranslator.cpp
    kcatalog.cpp
    kuitsetup.cpp
    common_helpers.cpp
    klocalizedcontext.cpp
    main.cpp
    klocalization.cpp
)

ecm_qt_declare_logging_category(KF6I18n
    HEADER ki18n_logging.h
    IDENTIFIER KI18N
    CATEGORY_NAME kf.i18n
    OLD_CATEGORY_NAMES kf5.ki18n
    DESCRIPTION "KI18n"
    EXPORT KI18N
)

ecm_qt_declare_logging_category(KF6I18n
    HEADER ki18n_logging_kuit.h
    IDENTIFIER KI18N_KUIT
    CATEGORY_NAME kf.i18n.kuit
    OLD_CATEGORY_NAMES kf5.ki18n.kuit
    DESCRIPTION "KI18n KUIT"
    EXPORT KI18N
)

ecm_generate_export_header(KF6I18n
    BASE_NAME KI18n
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 6.8
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF6I18n
    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KI18n>"
    PUBLIC "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>" # for version header
    PRIVATE ${LibIntl_INCLUDE_DIRS}
)

target_link_libraries(KF6I18n PUBLIC Qt6::Core)
# This is only required for platforms which don't use glibc (with glibc LibIntl_LIBRARIES will be empty)
target_link_libraries(KF6I18n PRIVATE ${LibIntl_LIBRARIES})
if (ANDROID)
    target_link_libraries(KF6I18n PRIVATE android)
endif()
target_compile_options(KF6I18n PRIVATE -DTRANSLATION_DOMAIN=\"ki18n6\")

install(TARGETS KF6I18n EXPORT KF6I18nTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

ecm_generate_headers(KI18n_HEADERS
  HEADER_NAMES
    KLazyLocalizedString
    KLocalizedContext
    KLocalizedString
    KLocalizedTranslator
    KLocalization
    KuitSetup
    REQUIRED_HEADERS KI18n_HEADERS
)

install(FILES
    ${KI18n_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/ki18n_export.h
    DESTINATION "${KDE_INSTALL_INCLUDEDIR_KF}/KI18n" COMPONENT Devel
)


### ktranscript plugin
if (BUILD_WITH_QML)

if (BUILD_SHARED_LIBS)
    add_library(ktranscript MODULE)
else()
    add_library(ktranscript)
endif()

target_sources(ktranscript PRIVATE
    ktranscript.cpp
    common_helpers.cpp
)
generate_export_header(ktranscript BASE_NAME KTranscript)
target_link_libraries(ktranscript PRIVATE Qt6::Qml Qt6::Core)

if (BUILD_SHARED_LIBS)
    install(TARGETS ktranscript DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6)
else()
    target_link_libraries(KF6I18n PRIVATE ktranscript)
    install(TARGETS ktranscript EXPORT KF6I18nTargets DESTINATION ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
endif()

endif()

ecm_generate_qdoc(KF6I18n ki18n.qdocconf)
