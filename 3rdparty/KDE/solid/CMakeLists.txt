cmake_minimum_required(VERSION 3.29)

set(KF_VERSION "6.25.0") # handled by release scripts
project(Solid VERSION ${KF_VERSION})

include(FeatureSummary)
find_package(ECM 6.25.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)


list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)

include(KDEGitCommitHooks)
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
include(ECMGenerateQDoc)

set(REQUIRED_QT_VERSION 6.9.0)
find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Xml Gui)


# shall we use DBus?
# enabled per default on Linux & BSD systems
# Android has no DBus, and Windows & Mac have backends that don't use DBus
set(USE_DBUS_DEFAULT OFF)
if(UNIX AND NOT APPLE AND NOT ANDROID AND NOT HAIKU)
    set(USE_DBUS_DEFAULT ON)
endif()
option(USE_DBUS "Build components using DBus" ${USE_DBUS_DEFAULT})
if(USE_DBUS)
    find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED DBus)
    set(HAVE_DBUS TRUE)
    add_definitions(-DHAVE_DBUS)
endif()

if(WIN32)
    find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Network)
endif()

include(ECMGenerateExportHeader)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMMarkNonGuiExecutable)

find_package(FLEX REQUIRED)
set_package_properties(FLEX PROPERTIES
    URL "http://flex.sourceforge.net"
    DESCRIPTION "Fast Lexical Analyzer"
    TYPE REQUIRED
    PURPOSE "Required for the Predicate parser"
)

find_package(BISON 3.0 REQUIRED)
set_package_properties(BISON PROPERTIES
    URL "http://www.gnu.org/software/bison"
    DESCRIPTION "general-purpose parser generator"
    TYPE REQUIRED
    PURPOSE "Required for the Predicate parser"
)

find_package(IMobileDevice)
set_package_properties(IMobileDevice PROPERTIES
                       TYPE OPTIONAL
                       PURPOSE "Needed to build the iOS device support backend"
                      )

find_package(PList)
set_package_properties(PList PROPERTIES
                       TYPE OPTIONAL
                       PURPOSE "Needed to build the iOS device support backend"
                      )

if (CMAKE_SYSTEM_NAME MATCHES Linux)
    # Used by the UDisks backend on Linux
    find_package(LibMount)
    set_package_properties(LibMount PROPERTIES
                           TYPE REQUIRED)
endif()

include(ECMPoQmTools)

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

set(solid_version_header "${CMAKE_CURRENT_BINARY_DIR}/src/solid/solid_version.h")
ecm_setup_version(PROJECT VARIABLE_PREFIX SOLID
                        VERSION_HEADER "${solid_version_header}"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6SolidConfigVersion.cmake"
                        SOVERSION 6)

ecm_set_disabled_deprecation_versions(
    QT 6.11.0
)

ecm_install_po_files_as_qm(poqm)

include(SolidBackendsMacros)

add_device_backends_begin()
add_device_backend(fakehw)
if (CMAKE_SYSTEM_NAME MATCHES Linux)
    option(UDEV_DISABLED "Allows disabling UDev usage on Linux builds" OFF)
    if (NOT UDEV_DISABLED)
        find_package(UDev REQUIRED)
        set_package_properties(UDev PROPERTIES
            TYPE REQUIRED
            PURPOSE "Allows Solid to use UDev to provide information about devices on Linux"
        )
    endif()
    if (UDev_FOUND)
        set(UDEV_FOUND TRUE) # for config-solid.h
        add_device_backend(udev)
    endif()
    set(HAVE_LIBMOUNT ${LibMount_FOUND})
    add_device_backend(fstab)
    if (HAVE_DBUS)
        if (UDEV_FOUND)
            add_device_backend(udisks2)
        endif()
        add_device_backend(upower)
    endif()
elseif (APPLE)
    find_package(IOKit REQUIRED)
    add_device_backend(iokit)
elseif (WIN32)
    add_device_backend(win)
elseif (ANDROID)
    # Android specific Storage and other device APIs not implemented
elseif (HAIKU)
    # Haiku OS specific Storage and other device APIs not implemented
else()
    # BSDs and other Unix like systems
    add_device_backend(fstab)

    if (HAVE_DBUS)
        add_device_backend(upower)
        if (CMAKE_SYSTEM_NAME MATCHES FreeBSD OR CMAKE_SYSTEM_NAME MATCHES OpenBSD)
            add_device_backend(udisks2)
        endif()
    endif()
endif()

if(IMobileDevice_FOUND AND PList_FOUND)
    add_device_backend(imobile)
endif()

add_device_backends_cmake()

add_subdirectory(src)
if (BUILD_TESTING)
    add_subdirectory(autotests)
endif()

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6Solid")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KF6SolidConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KF6SolidConfig.cmake"
    PATH_VARS KDE_INSTALL_DBUSINTERFACEDIR
    INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KF6SolidConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KF6SolidConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(EXPORT KF6SolidTargets  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"  FILE KF6SolidTargets.cmake  NAMESPACE KF6:: )

install(FILES ${solid_version_header}
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/Solid COMPONENT Devel
)

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
