# SPDX-FileCopyrightText: 2024 Manuel Alcaraz Zambrano <manuelalcarazzam@gmail.com>
# SPDX-License-Identifier: BSD-2-Clause

set(bindings_library "KGuiAddons")

set(wrapped_header ${CMAKE_CURRENT_SOURCE_DIR}/bindings.h)
set(typesystem_file ${CMAKE_CURRENT_SOURCE_DIR}/bindings.xml)

set(generated_sources
    ${CMAKE_CURRENT_BINARY_DIR}/KGuiAddons/kguiaddons_module_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KGuiAddons/kcolormimedata_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KGuiAddons/kcolorschemewatcher_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KGuiAddons/kcolorutils_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KGuiAddons/kcountryflagemojiiconengine_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KGuiAddons/kcursorsaver_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KGuiAddons/kdatevalidator_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KGuiAddons/kfontutils_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KGuiAddons/kiconutils_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KGuiAddons/kjobwindows_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KGuiAddons/kkeysequencerecorder_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KGuiAddons/kmodifierkeyinfo_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KGuiAddons/ksystemclipboard_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KGuiAddons/kwordwrap_wrapper.cpp)

ecm_generate_python_bindings(
    PACKAGE_NAME ${bindings_library}
    VERSION ${KF_VERSION}
    WRAPPED_HEADER ${wrapped_header}
    TYPESYSTEM ${typesystem_file}
    GENERATED_SOURCES ${generated_sources}
    DEPENDENCIES KF6::GuiAddons
    QT_VERSION ${REQUIRED_QT_VERSION}
    HOMEPAGE_URL "https://invent.kde.org/frameworks/kguiaddons"
    ISSUES_URL "https://bugs.kde.org/describecomponents.cgi?product=frameworks-kguiaddons"
    AUTHOR "The KDE Community"
    README ../README.md
)

target_link_libraries(${bindings_library} PRIVATE KF6::GuiAddons)

execute_process(COMMAND ${Python_EXECUTABLE} -Esc "import sysconfig; print(sysconfig.get_path('platlib', vars={'platbase': '${CMAKE_INSTALL_PREFIX}', 'base': '${CMAKE_INSTALL_PREFIX}'}))" OUTPUT_VARIABLE sysconfig_output)
string(STRIP ${sysconfig_output} PYTHON_INSTALL_DIR)

install(TARGETS ${bindings_library} LIBRARY DESTINATION "${PYTHON_INSTALL_DIR}")
