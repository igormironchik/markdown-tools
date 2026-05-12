# SPDX-FileCopyrightText: 2024 Manuel Alcaraz Zambrano <manuelalcarazzam@gmail.com>
# SPDX-License-Identifier: BSD-2-Clause

set(bindings_library "KCoreAddons")

set(wrapped_header ${CMAKE_CURRENT_SOURCE_DIR}/bindings.h)
set(typesystem_file ${CMAKE_CURRENT_SOURCE_DIR}/typesystem_kcoreaddons.xml)

set(generated_sources
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kcoreaddons_module_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kcoreaddons_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kfuzzymatcher_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kfuzzymatcher_range_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kfuzzymatcher_result_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kprocesslist_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kprocesslist_kprocessinfo_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/ksandbox_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/ksandbox_processcontext_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kaboutcomponent_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kaboutdata_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kaboutlicense_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kaboutperson_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kautosavefile_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kbackup_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kcharmacroexpander_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kcompositejob_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kdirwatch_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kfilesystemtype_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kfileutils_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kformat_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kgroupid_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kjob_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kjobtrackerinterface_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kjobuidelegate_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kjsonutils_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/klibexec_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/klistopenfilesjob_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kmacroexpander_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kmacroexpanderbase_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kmemoryinfo_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/knetworkmounts_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kosrelease_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kpluginfactory_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kpluginmetadata_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kprocess_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/krandom_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kruntimeplatform_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kshareddatacache_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kshell_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/ksignalhandler_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kstringhandler_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/ktexttohtml_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kurlmimedata_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kuser_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kusergroup_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kuserid_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons/kwordmacroexpander_wrapper.cpp)

ecm_generate_python_bindings(
    PACKAGE_NAME ${bindings_library}
    VERSION ${KF_VERSION}
    WRAPPED_HEADER ${wrapped_header}
    TYPESYSTEM ${typesystem_file}
    EXPORT_TYPESYSTEM
    GENERATED_SOURCES ${generated_sources}
    DEPENDENCIES KF6::CoreAddons
    QT_VERSION ${REQUIRED_QT_VERSION}
    HOMEPAGE_URL "https://invent.kde.org/frameworks/kcoreaddons"
    ISSUES_URL "https://bugs.kde.org/describecomponents.cgi?product=frameworks-kcoreaddons"
    AUTHOR "The KDE Community"
    README ../README.md
)

target_link_libraries(${bindings_library} PRIVATE KF6CoreAddons)

execute_process(COMMAND ${Python_EXECUTABLE} -Esc "import sysconfig; print(sysconfig.get_path('platlib', vars={'platbase': '${CMAKE_INSTALL_PREFIX}', 'base': '${CMAKE_INSTALL_PREFIX}'}))" OUTPUT_VARIABLE sysconfig_output)
string(STRIP ${sysconfig_output} PYTHON_INSTALL_DIR)

install(TARGETS ${bindings_library} LIBRARY DESTINATION "${PYTHON_INSTALL_DIR}")
