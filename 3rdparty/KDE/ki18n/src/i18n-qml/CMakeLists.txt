# SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_library(KF6I18nQml)
add_library(KF6::I18nQml ALIAS KF6I18nQml)

ecm_add_qml_module(KF6I18nQml URI "org.kde.ki18n" VERSION 1.0 GENERATE_PLUGIN_SOURCE)

set_target_properties(KF6I18nQml PROPERTIES
    VERSION     ${KI18N_VERSION}
    SOVERSION   ${KI18N_SOVERSION}
    EXPORT_NAME I18nQml
)

target_sources(KF6I18nQml PRIVATE
    klocalizedqmlcontext.cpp
    localizedcontext.cpp
)

ecm_qt_declare_logging_category(KF6I18nQml
    HEADER ki18n_qml_logging.h
    IDENTIFIER KI18N
    CATEGORY_NAME kf.i18n.qml
    DESCRIPTION "KI18n QML Integration"
    EXPORT KI18N
)

ecm_generate_export_header(KF6I18nQml
    BASE_NAME KI18nQml
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    VERSION_BASE_NAME KI18n
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF6I18nQml
    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KI18n>"
    PUBLIC "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>" # for version header
)

target_link_libraries(KF6I18nQml
    PUBLIC
        Qt6::Core
    PRIVATE
        KF6I18n
        Qt6::Qml
)

ecm_generate_headers(KI18n_HEADERS
  HEADER_NAMES
    KLocalizedQmlContext
    REQUIRED_HEADERS KI18n_HEADERS
)

ecm_generate_qdoc(KF6I18nQml ki18nqml.qdocconf)
ecm_finalize_qml_module(KF6I18nQml DESTINATION ${KDE_INSTALL_QMLDIR} EXPORT KF6I18nTargets)

install(TARGETS KF6I18nQml EXPORT KF6I18nTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ${KI18n_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/ki18nqml_export.h
    DESTINATION "${KDE_INSTALL_INCLUDEDIR_KF}/KI18n" COMPONENT Devel
)

