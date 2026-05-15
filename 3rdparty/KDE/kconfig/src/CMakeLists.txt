add_subdirectory(core)
if(TARGET Qt6::Gui)
    add_subdirectory(gui)
endif()
if(KCONFIG_USE_QML)
    add_subdirectory(qml)
endif()
add_subdirectory(kconfig_compiler)
add_subdirectory(kconf_update)
add_subdirectory(kreadconfig)

ecm_qt_install_logging_categories(
    EXPORT KCONFIG
    FILE kconfig.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
