# SPDX-FileCopyrightText: 2025 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: BSD-2-Clause

set(bindings_library "KJobWidgets")

set(wrapped_header ${CMAKE_CURRENT_SOURCE_DIR}/bindings.h)
set(typesystem_file ${CMAKE_CURRENT_SOURCE_DIR}/typesystem_kjobwidgets.xml)

set(generated_sources
    ${CMAKE_CURRENT_BINARY_DIR}/KJobWidgets/kjobwidgets_module_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KJobWidgets/kabstractwidgetjobtracker_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KJobWidgets/kdialogjobuidelegate_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KJobWidgets/knotificationjobuidelegate_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KJobWidgets/kstatusbarjobtracker_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KJobWidgets/kuiserverjobtracker_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KJobWidgets/kuiserverv2jobtracker_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KJobWidgets/kwidgetjobtracker_wrapper.cpp
)

ecm_generate_python_bindings(
    PACKAGE_NAME ${bindings_library}
    VERSION ${KF_VERSION}
    WRAPPED_HEADER ${wrapped_header}
    TYPESYSTEM ${typesystem_file}
    GENERATED_SOURCES ${generated_sources}
    DEPENDENCIES KF6::JobWidgets
    QT_VERSION ${REQUIRED_QT_VERSION}
    HOMEPAGE_URL "https://invent.kde.org/frameworks/kjobwidgets"
    ISSUES_URL "https://bugs.kde.org/describecomponents.cgi?product=frameworks-kjobwidgets"
    AUTHOR "The KDE Community"
    README ${CMAKE_SOURCE_DIR}/README.md
)

target_link_libraries(${bindings_library} PRIVATE KF6::JobWidgets)

execute_process(COMMAND ${Python_EXECUTABLE} -Esc "import sysconfig; print(sysconfig.get_path('platlib', vars={'platbase': '${CMAKE_INSTALL_PREFIX}', 'base': '${CMAKE_INSTALL_PREFIX}'}))" OUTPUT_VARIABLE sysconfig_output)
string(STRIP ${sysconfig_output} PYTHON_INSTALL_DIR)

install(TARGETS ${bindings_library} LIBRARY DESTINATION "${PYTHON_INSTALL_DIR}")
