# SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
# SPDX-License-Identifier: BSD-3-Clause

ecm_add_qml_module(kcmutilsqmlplugin URI "org.kde.kcmutils" VERSION 1.0 DEPENDENCIES QtQuick org.kde.kcmutils.private org.kde.config GENERATE_PLUGIN_SOURCE)

ecm_generate_qdoc(kcmutilsqmlplugin kcmutilsqml.qdocconf)

target_sources(kcmutilsqmlplugin
  PRIVATE
    settingstateproxy.cpp
    kcmlauncher.cpp
    types.h
)
target_link_libraries(kcmutilsqmlplugin
  PRIVATE
    KF6::KIOGui
    Qt6::Qml
    Qt6::Quick
    kcmutils_proxy_model
    kcmutils_logging_STATIC
    KF6KCMUtilsQuick
)

ecm_target_qml_sources(kcmutilsqmlplugin SOURCES
    components/PluginDelegate.qml
    components/PluginSelector.qml
    components/AbstractKCM.qml
    components/ContextualHelpButton.qml
    components/GridDelegate.qml
    components/GridView.qml
    components/GridViewKCM.qml
    components/ScrollView.qml
    components/ScrollViewKCM.qml
    components/SettingHighlighter.qml
    components/SettingStateBinding.qml
    components/SimpleKCM.qml
)
ecm_target_qml_sources(kcmutilsqmlplugin PATH private SOURCES
    components/private/AboutPlugin.qml
    components/private/GridDelegateMenu.qml
    components/private/GridViewInternal.qml
)
ecm_finalize_qml_module(kcmutilsqmlplugin DESTINATION ${KDE_INSTALL_QMLDIR})

ecm_add_qml_module(kcmutilsprivateqmlplugin URI "org.kde.kcmutils.private" DEPENDENCIES QtCore QtQuick GENERATE_PLUGIN_SOURCE)
target_sources(kcmutilsprivateqmlplugin PRIVATE private_types.h settinghighlighterprivate.cpp)
target_link_libraries(kcmutilsprivateqmlplugin PRIVATE Qt6::Quick kcmutils_proxy_model)
ecm_finalize_qml_module(kcmutilsprivateqmlplugin DESTINATION ${KDE_INSTALL_QMLDIR})

