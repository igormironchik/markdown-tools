# SPDX-FileCopyrightText: 2024 Manuel Alcaraz Zambrano <manuelalcarazzam@gmail.com>
# SPDX-License-Identifier: BSD-2-Clause

set(bindings_library "KWidgetsAddons")

set(wrapped_header ${CMAKE_SOURCE_DIR}/python/bindings.h)
set(typesystem_file ${CMAKE_SOURCE_DIR}/python/bindings.xml)

set(generated_sources
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kwidgetsaddons_module_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kacceleratormanager_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kactionmenu_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kactionselector_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kanimatedbutton_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kassistantdialog_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kbusyindicatorwidget_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kcapacitybar_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kcharselect_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kcollapsiblegroupbox_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kcolorbutton_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kcolorcombo_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kcolumnresizer_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kcontextualhelpbutton_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kcursor_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kdatecombobox_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kdatepicker_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kdatepickerpopup_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kdatetimeedit_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kdragwidgetdecoratorbase_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kdualaction_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/keditlistwidget_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kfontaction_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kfontchooser_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kfontchooserdialog_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kfontrequester_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kfontsizeaction_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kgradientselector_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kguiitem_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kjobwidgets_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kled_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/klineediteventhandler_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/klineediturldropeventfilter_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kmessagebox_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kmessageboxnotifyinterface_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kmessagedialog_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kmessagewidget_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kmimetypechooser_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kmimetypechooserdialog_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kmimetypeeditor_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kmultitabbar_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kmultitabbarbutton_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kmultitabbartab_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/knewpassworddialog_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/knewpasswordwidget_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kpagedialog_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kpagemodel_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kpageview_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kpagewidget_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kpagewidgetitem_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kpagewidgetmodel_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kpassword_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kpassworddialog_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kpasswordlineedit_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kpixmapregionselectordialog_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kpixmapregionselectorwidget_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kpixmapsequence_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kpixmapsequenceoverlaypainter_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kpixmapsequencewidget_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kpopupframe_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kratingpainter_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kratingwidget_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/krecentfilesmenu_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kruler_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kselectaction_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kselector_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kseparator_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/ksplittercollapserbutton_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kstandardguiitem_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kstyleextensions_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/ksqueezedtextlabel_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/ktimecombobox_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/ktitlewidget_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/ktoggleaction_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/ktogglefullscreenaction_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/ktoolbarlabelaction_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/ktoolbarpopupaction_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/ktoolbarspaceraction_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/ktooltipwidget_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/ktwofingerswipe_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/ktwofingerswiperecognizer_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/ktwofingertap_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/ktwofingertaprecognizer_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kurllabel_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kviewstateserializer_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KWidgetsAddons/kxyselector_wrapper.cpp)

ecm_generate_python_bindings(
    PACKAGE_NAME ${bindings_library}
    VERSION ${KF_VERSION}
    WRAPPED_HEADER ${wrapped_header}
    TYPESYSTEM ${typesystem_file}
    GENERATED_SOURCES ${generated_sources}
    DEPENDENCIES KF6::WidgetsAddons
    QT_VERSION ${REQUIRED_QT_VERSION}
    HOMEPAGE_URL "https://invent.kde.org/frameworks/kwidgetsaddons"
    ISSUES_URL "https://bugs.kde.org/describecomponents.cgi?product=frameworks-kwidgetsaddons"
    AUTHOR "The KDE Community"
    README ../README.md
)

target_link_libraries(${bindings_library} PRIVATE KF6WidgetsAddons)

execute_process(COMMAND ${Python_EXECUTABLE} -Esc "import sysconfig; print(sysconfig.get_path('platlib', vars={'platbase': '${CMAKE_INSTALL_PREFIX}', 'base': '${CMAKE_INSTALL_PREFIX}'}))" OUTPUT_VARIABLE sysconfig_output)

string(STRIP ${sysconfig_output} PYTHON_INSTALL_DIR)


install(TARGETS ${bindings_library} LIBRARY DESTINATION "${PYTHON_INSTALL_DIR}")
