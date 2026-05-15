# SPDX-FileCopyrightText: 2024 Manuel Alcaraz Zambrano <manuelalcarazzam@gmail.com>
# SPDX-License-Identifier: BSD-2-Clause

set(bindings_library "KXmlGui")

set(wrapped_header ${CMAKE_CURRENT_SOURCE_DIR}/bindings.h)
set(typesystem_file ${CMAKE_CURRENT_SOURCE_DIR}/bindings.xml)

set(generated_sources
    ${CMAKE_CURRENT_BINARY_DIR}/KXmlGui/kxmlgui_module_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KXmlGui/kaboutapplicationdialog_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KXmlGui/kaboutplugindialog_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KXmlGui/kactioncategory_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KXmlGui/kactioncollection_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KXmlGui/kbugreport_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KXmlGui/kedittoolbar_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KXmlGui/khelpmenu_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KXmlGui/kkeysequencewidget_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KXmlGui/kmainwindow_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KXmlGui/kshortcutsdialog_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KXmlGui/kshortcutseditor_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KXmlGui/kshortcutwidget_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KXmlGui/ktoggletoolbaraction_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KXmlGui/ktoolbar_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KXmlGui/ktooltiphelper_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KXmlGui/kundoactions_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KXmlGui/kxmlguibuilder_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KXmlGui/kxmlguiclient_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KXmlGui/kxmlguifactory_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KXmlGui/kxmlguiwindow_wrapper.cpp)

ecm_generate_python_bindings(
    PACKAGE_NAME ${bindings_library}
    VERSION ${KF_VERSION}
    WRAPPED_HEADER ${wrapped_header}
    TYPESYSTEM ${typesystem_file}
    GENERATED_SOURCES ${generated_sources}
    DEPENDENCIES KF6::XmlGui
    QT_VERSION ${REQUIRED_QT_VERSION}
    HOMEPAGE_URL "https://invent.kde.org/frameworks/kxmlgui"
    ISSUES_URL "https://bugs.kde.org/describecomponents.cgi?product=frameworks-kxmlgui"
    AUTHOR "The KDE Community"
    README ../README.md
)

target_link_libraries(${bindings_library} PRIVATE KF6::XmlGui)

execute_process(COMMAND ${Python_EXECUTABLE} -Esc "import sysconfig; print(sysconfig.get_path('platlib', vars={'platbase': '${CMAKE_INSTALL_PREFIX}', 'base': '${CMAKE_INSTALL_PREFIX}'}))" OUTPUT_VARIABLE sysconfig_output)

string(STRIP ${sysconfig_output} PYTHON_INSTALL_DIR)

install(TARGETS ${bindings_library} LIBRARY DESTINATION "${PYTHON_INSTALL_DIR}")

