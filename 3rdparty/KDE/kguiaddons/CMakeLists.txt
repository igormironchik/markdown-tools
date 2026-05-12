cmake_minimum_required(VERSION 3.29)

set(KF_VERSION "6.25.0") # handled by release scripts
project(KGuiAddons VERSION ${KF_VERSION})

include(FeatureSummary)
find_package(ECM 6.25.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

include(KDEGitCommitHooks)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
include(ECMGenerateExportHeader)
include(ECMGeneratePkgConfigFile)
include(ECMMarkNonGuiExecutable)
include(ECMQmlModule)
include(ECMGenerateQDoc)

option(BUILD_GEO_SCHEME_HANDLER "Build the geo: URI scheme handler. Disable this if you are building two major versions for co-installability in the older variant." ON)

if(UNIX AND NOT APPLE AND NOT ANDROID AND NOT HAIKU)
    option(WITH_WAYLAND "Build with support for KModifierKeyInfo, KSystemClipboard and KKeySequenceRecorder inhibiting shortcuts on Wayland" ON)
    add_feature_info(WAYLAND ${WITH_WAYLAND} "KeySequenceEditor inhibiting shortcuts on Wayland")

    option(WITH_X11 "Build with support for KModifierKeyInfo on X11" ON)
    add_feature_info(X11 ${WITH_X11} "KeySequenceEditor inhibiting shortcuts on X11")
else()
    set(WITH_WAYLAND OFF)
    set(WITH_X11 OFF)
endif()

set(REQUIRED_QT_VERSION 6.9.0)

find_package(Qt6Gui ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)

if (WITH_WAYLAND)
    # Qt6GuiPrivate  must be found before Qt6WaylandClient
    if (Qt6Gui_VERSION VERSION_GREATER_EQUAL "6.10.0")
        find_package(Qt6GuiPrivate ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
    else()
        find_package(Qt6Gui ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS Private)
    endif()
endif()

if (WITH_X11)
    find_package(X11 REQUIRED MODULE)
    find_package(XCB REQUIRED MODULE COMPONENTS XCB)
    if(NOT QT_FEATURE_xcb)
        message(FATAL_ERROR "Qt xcb feature is missing for compilation with X11")
    endif()
endif()

if (WITH_WAYLAND)
    find_package(WaylandProtocols 1.39 REQUIRED)
    find_package(PlasmaWaylandProtocols 1.15.0 REQUIRED)
    find_package(Qt6WaylandClient ${REQUIRED_QT_VERSION} REQUIRED CONFIG)
    find_package(Wayland 1.9 REQUIRED Client)
endif()

# shall we use DBus?
# enabled per default on Linux & BSD systems
set(USE_DBUS_DEFAULT OFF)
if(UNIX AND NOT APPLE AND NOT ANDROID AND NOT HAIKU)
    set(USE_DBUS_DEFAULT ON)
endif()
option(USE_DBUS "Build components using DBus (get color scheme information via XDG portals)" ${USE_DBUS_DEFAULT})
if(USE_DBUS)
    find_package(Qt6DBus ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
    if (Qt6Gui_VERSION VERSION_GREATER_EQUAL "6.10.0")
        find_package(Qt6GuiPrivate ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
    else()
        find_package(Qt6Gui ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS Private)
    endif()
    set(WITH_DBUS ON)
endif()

if (ANDROID)
    find_package(Gradle REQUIRED)
endif()
option(BUILD_PYTHON_BINDINGS "Build Python bindings" ON)

# Only Linux and FreeBSD CI has the relevant packages
if (ANDROID OR APPLE OR WIN32 OR HAIKU OR NOT BUILD_SHARED_LIBS)
    set(BUILD_PYTHON_BINDINGS OFF)
endif()

if (BUILD_PYTHON_BINDINGS)
    find_package(Python3 3.9 REQUIRED COMPONENTS Interpreter Development)
    find_package(Shiboken6 REQUIRED)
    find_package(PySide6 REQUIRED)
endif()

add_feature_info(PYTHON_BINDINGS ${BUILD_PYTHON_BINDINGS} "Python bindings")

set(kguiaddons_version_header "${CMAKE_CURRENT_BINARY_DIR}/src/kguiaddons_version.h")
ecm_setup_version(PROJECT VARIABLE_PREFIX KGUIADDONS
                        VERSION_HEADER "${kguiaddons_version_header}"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6GuiAddonsConfigVersion.cmake"
                        SOVERSION 6)

ecm_set_disabled_deprecation_versions(
    QT 6.11
)

add_subdirectory(src)
if (BUILD_TESTING)
    add_subdirectory(autotests)
    add_subdirectory(tests)
endif()

if (BUILD_PYTHON_BINDINGS)
    include(ECMGeneratePythonBindings)
    add_subdirectory(python)
endif()

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6GuiAddons")

include(CMakePackageConfigHelpers)

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KF6GuiAddonsConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KF6GuiAddonsConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KF6GuiAddonsConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KF6GuiAddonsConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )


install(EXPORT KF6GuiAddonsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE KF6GuiAddonsTargets.cmake NAMESPACE KF6:: COMPONENT Devel)


install(FILES ${kguiaddons_version_header}
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KGuiAddons COMPONENT Devel)

if (NOT WIN32)
    ecm_generate_pkgconfig_file(BASE_NAME KF6GuiAddons
      INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF}/KGuiAddons
      DEPS Qt6Gui
    INSTALL)
endif()

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
