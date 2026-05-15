# SPDX-FileCopyrightText: 2024 Manuel Alcaraz Zambrano <manuelalcarazzam@gmail.com>
# SPDX-License-Identifier: BSD-2-Clause

set(bindings_library "KNotifications")

set(wrapped_header ${CMAKE_CURRENT_SOURCE_DIR}/bindings.h)
set(typesystem_file ${CMAKE_CURRENT_SOURCE_DIR}/bindings.xml)

set(generated_sources
    ${CMAKE_CURRENT_BINARY_DIR}/KNotifications/knotifications_module_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KNotifications/knotification_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KNotifications/knotificationaction_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KNotifications/knotificationreplyaction_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KNotifications/knotifyconfig_wrapper.cpp)

set(dependencies Qt6::Core Qt6::Gui KF6::Notifications)

ecm_generate_python_bindings(
    PACKAGE_NAME ${bindings_library}
    VERSION ${KF_VERSION}
    WRAPPED_HEADER ${wrapped_header}
    TYPESYSTEM ${typesystem_file}
    GENERATED_SOURCES ${generated_sources}
    DEPENDENCIES ${dependencies}
    QT_VERSION ${REQUIRED_QT_VERSION}
    HOMEPAGE_URL "https://invent.kde.org/frameworks/knotifications"
    ISSUES_URL "https://bugs.kde.org/describecomponents.cgi?product=frameworks-knotifications"
    AUTHOR "The KDE Community"
    README ../README.md
)

target_link_libraries(${bindings_library} PRIVATE KF6Notifications)

execute_process(COMMAND ${Python_EXECUTABLE} -Esc "import sysconfig; print(sysconfig.get_path('platlib', vars={'platbase': '${CMAKE_INSTALL_PREFIX}', 'base': '${CMAKE_INSTALL_PREFIX}'}))" OUTPUT_VARIABLE sysconfig_output)
string(STRIP ${sysconfig_output} PYTHON_INSTALL_DIR)

install(TARGETS ${bindings_library} LIBRARY DESTINATION "${PYTHON_INSTALL_DIR}")
