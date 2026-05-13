cmake_minimum_required(VERSION 3.29)

set(KF_VERSION "6.25.0") # handled by release scripts
project(KI18n VERSION ${KF_VERSION})

# ECM setup
include(FeatureSummary)
find_package(ECM 6.25.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${KI18n_SOURCE_DIR}/cmake ${KI18n_BINARY_DIR}/cmake)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEGitCommitHooks)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

include(ECMGenerateExportHeader)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
include(ECMQmlModule)
include(ECMGenerateQDoc)


ecm_setup_version(
  PROJECT
  VARIABLE_PREFIX KI18N
  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/ki18n_version.h"
  PACKAGE_VERSION_FILE "${CMAKE_BINARY_DIR}/cmake/KF6I18nConfigVersion.cmake"
  SOVERSION 6)

# Dependencies
set(REQUIRED_QT_VERSION 6.9.0)

find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Core OPTIONAL_COMPONENTS Widgets)

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

option(BUILD_WITH_QML "Build with support for scripted translations (recommended)" ON)
if(BUILD_WITH_QML)
    find_package(Qt6Qml ${REQUIRED_QT_VERSION} CONFIG REQUIRED)
endif()

find_package(LibIntl)
set_package_properties(LibIntl PROPERTIES TYPE REQUIRED
    URL "http://gnuwin32.sourceforge.net/packages/libintl.htm"
    PURPOSE "Needed for building KI18n unless glibc is the system libc implementation"
)

find_package(IsoCodes)
set_package_properties(IsoCodes PROPERTIES TYPE RUNTIME
    PURPOSE "Translation catalogs for countries, country subdivisions, languages and currencies"
)

# KF6I18nMacros.cmake only needs to know the python executable path.
# Due to CMake caching the variables, and KF6I18nMacros being included by KF6I18nConfig,
# we have to hardcode the PYTHON_EXECUTABLE path or anything depending on KF6I18n
# would be unable to find another Python version.
find_package(Python3 REQUIRED COMPONENTS Interpreter)
# allow settings this path explicitly for cross-building setups
if (NOT FALLBACK_KI18N_PYTHON_EXECUTABLE)
    set(FALLBACK_KI18N_PYTHON_EXECUTABLE ${Python3_EXECUTABLE})
endif()
configure_file(cmake/KF6I18nMacros.cmake.in ${KI18n_BINARY_DIR}/cmake/KF6I18nMacros.cmake @ONLY)
# Needed to build the ki18n translations and run the autotests
file(COPY ${KI18n_SOURCE_DIR}/cmake/build-pofiles.cmake DESTINATION ${KI18n_BINARY_DIR}/cmake)
file(COPY ${KI18n_SOURCE_DIR}/cmake/build-tsfiles.cmake DESTINATION ${KI18n_BINARY_DIR}/cmake)
file(COPY ${KI18n_SOURCE_DIR}/cmake/ts-pmap-compile.py DESTINATION ${KI18n_BINARY_DIR}/cmake)
file(COPY ${KI18n_SOURCE_DIR}/cmake/kf6i18nuic.cmake DESTINATION ${KI18n_BINARY_DIR}/cmake)

# Create dummy file to execute find_package(KF6I18n) within autotests/ki18n_install
file(WRITE ${CMAKE_BINARY_DIR}/cmake/KF6I18nTargets.cmake "")

include(${KI18n_BINARY_DIR}/cmake/KF6I18nMacros.cmake)

remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
if(MSVC)
    remove_definitions(-DQT_STRICT_ITERATORS)
endif()

ecm_set_disabled_deprecation_versions(
    QT 6.11.0
)

ki18n_install(po)
add_subdirectory(src)
if(BUILD_TESTING)
    add_subdirectory(autotests)
    add_subdirectory(tests)
endif()

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6I18n")

configure_package_config_file("${CMAKE_CURRENT_LIST_DIR}/KF6I18nConfig.cmake.in"
                              "${CMAKE_BINARY_DIR}/cmake/KF6I18nConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES  "${CMAKE_BINARY_DIR}/cmake/KF6I18nConfig.cmake"
    "${CMAKE_BINARY_DIR}/cmake/KF6I18nConfigVersion.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    COMPONENT Devel)

install(EXPORT KF6I18nTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF6I18nTargets.cmake NAMESPACE KF6::)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ki18n_version.h
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KI18n COMPONENT Devel)

install( FILES
    ${KI18n_BINARY_DIR}/cmake/KF6I18nMacros.cmake
    cmake/kf6i18nuic.cmake
    cmake/build-pofiles.cmake
    cmake/build-tsfiles.cmake
    cmake/ts-pmap-compile.py
    DESTINATION ${CMAKECONFIG_INSTALL_DIR} COMPONENT Devel )

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
