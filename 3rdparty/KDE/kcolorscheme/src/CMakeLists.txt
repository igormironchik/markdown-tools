# SPDX-FileCopyrightText: 2023 David Redondo <kde@david-redondo.de>
# SPDX-License-Identifier: BSD-2-Clause

add_library(KF6ColorScheme)
add_library(KF6::ColorScheme ALIAS KF6ColorScheme)

set_target_properties(KF6ColorScheme PROPERTIES
    VERSION     ${KCOLORSCHEME_VERSION}
    SOVERSION   ${KCOLORSCHEME_SOVERSION}
    EXPORT_NAME ColorScheme
)

target_sources(KF6ColorScheme PRIVATE
  kcolorscheme.cpp
  kcolorschememanager.cpp
  kcolorschememodel.cpp
  kstatefulbrush.cpp
)

qt_add_resources(KF6ColorScheme "color_schemes"
    PREFIX "/org.kde.kcolorscheme"
    FILES
        color-schemes/BreezeDark.colors
        color-schemes/BreezeLight.colors
    OUTPUT_TARGETS _rcc_targets
)
install(TARGETS ${_rcc_targets} EXPORT KF6ColorSchemeTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

ecm_qt_declare_logging_category(KF6ColorScheme
    HEADER kcolorscheme_debug.h
    IDENTIFIER KCOLORSCHEME
    CATEGORY_NAME kf.colorscheme
    DESCRIPTION "KColorScheme"
    EXPORT KCOLORSCHEME
)

ecm_generate_export_header(KF6ColorScheme
    BASE_NAME KColorScheme
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 6.6 6.19
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF6ColorScheme INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KColorScheme>")

target_link_libraries(KF6ColorScheme
  PUBLIC
    KF6::ConfigCore
    Qt6::Gui
  PRIVATE
    Qt6::GuiPrivate
    KF6::ConfigGui # ### this one is not directly used but has to be loaded in order for QColor I/O from KConfig to work!
    KF6::GuiAddons # KColorScheme uses KColorUtils
    KF6::I18n # For action and widget texts
)

ecm_generate_headers(KColorScheme_HEADERS
  HEADER_NAMES
  KColorScheme
  KColorSchemeManager
  KColorSchemeModel
  KStatefulBrush

  REQUIRED_HEADERS KColorScheme_HEADERS
)

install(TARGETS KF6ColorScheme EXPORT KF6ColorSchemeTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kcolorscheme_export.h
  ${KColorScheme_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KColorScheme COMPONENT Devel
)

ecm_qt_install_logging_categories(
    EXPORT KCOLORSCHEME
    FILE kcolorscheme.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

ecm_generate_qdoc(KF6ColorScheme kcolorscheme.qdocconf)
