configure_file(config-kjobwidgets.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kjobwidgets.h )

add_library(KF6JobWidgets)
add_library(KF6::JobWidgets ALIAS KF6JobWidgets)

set_target_properties(KF6JobWidgets PROPERTIES
    VERSION     ${KJOBWIDGETS_VERSION}
    SOVERSION   ${KJOBWIDGETS_SOVERSION}
    EXPORT_NAME JobWidgets
)

ecm_create_qm_loader(KF6JobWidgets kjobwidgets6_qt)

set(kjobwidgets_dbus_SRCS)
if (HAVE_QTDBUS)
    qt_add_dbus_interfaces(kjobwidgets_dbus_SRCS dbus/org.kde.JobViewServer.xml )
    qt_add_dbus_interface(kjobwidgets_dbus_SRCS dbus/org.kde.JobView.xml jobviewiface )

    qt_add_dbus_interfaces(kjobwidgets_dbus_SRCS dbus/org.kde.JobViewServerV2.xml )
    qt_add_dbus_interface(kjobwidgets_dbus_SRCS dbus/org.kde.JobViewV3.xml jobviewv3iface )

    qt_add_dbus_interface(kjobwidgets_dbus_SRCS dbus/org.freedesktop.PowerManagement.Inhibit.xml inhibit_interface)
    qt_add_dbus_interface(kjobwidgets_dbus_SRCS dbus/org.freedesktop.portal.Inhibit.xml portal_inhibit_interface)
    qt_add_dbus_interface(kjobwidgets_dbus_SRCS dbus/org.freedesktop.portal.Request.xml portal_request_interface)
endif()

target_sources(KF6JobWidgets PRIVATE
    kabstractwidgetjobtracker.cpp
    kabstractwidgetjobtracker.h
    kabstractwidgetjobtracker_p.h
    kdialogjobuidelegate.cpp
    kdialogjobuidelegate.h
    kinhibitionjobtracker.cpp
    kinhibitionjobtracker.h
    kinhibitionjobtracker_p.h
    kjobtrackerformatters.cpp
    kjobtrackerformatters_p.h
    kstatusbarjobtracker.cpp
    kstatusbarjobtracker.h
    kstatusbarjobtracker_p.h
    kwidgetjobtracker.cpp
    kwidgetjobtracker.h
    kwidgetjobtracker_p.h
    knotificationjobuidelegate.cpp
    knotificationjobuidelegate.h

    ${kjobwidgets_dbus_SRCS}
)
if (HAVE_QTDBUS)
    target_sources(KF6JobWidgets PRIVATE
        kuiserverjobtracker.cpp
        kuiserverjobtracker.h
        kuiserverjobtracker_p.h
        kuiserverv2jobtracker.cpp
        kuiserverv2jobtracker.h
        kuiserverv2jobtracker_p.h
    )
endif()

ecm_qt_declare_logging_category(KF6JobWidgets
    HEADER debug.h
    IDENTIFIER KJOBWIDGETS
    CATEGORY_NAME kf.jobwidgets
    OLD_CATEGORY_NAMES kf5.kjobwidgets
    DESCRIPTION "KJobWidgets"
    EXPORT KJOBWIDGETS
)

ecm_generate_export_header(KF6JobWidgets
    BASE_NAME KJobWidgets
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 6.17
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF6JobWidgets INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KJobWidgets>")

target_link_libraries(KF6JobWidgets
   PUBLIC
      Qt6::Widgets
      KF6::CoreAddons    # KJob
   PRIVATE
      KF6::WidgetsAddons # KSqueezedTextLabel
      KF6::Notifications
)
if (HAVE_QTDBUS)
    target_link_libraries(KF6JobWidgets PRIVATE Qt6::DBus)
endif()

if (WITH_X11)
    target_link_libraries(KF6JobWidgets PRIVATE Qt6::GuiPrivate) # qtx11extras_p.h
endif ()

ecm_generate_headers(KJobWidgets_HEADERS
  HEADER_NAMES
  KAbstractWidgetJobTracker
  KDialogJobUiDelegate
  KInhibitionJobTracker
  KUiServerJobTracker
  KUiServerV2JobTracker
  KStatusBarJobTracker
  KWidgetJobTracker
  KNotificationJobUiDelegate

  REQUIRED_HEADERS KJobWidgets_HEADERS
)

install(TARGETS KF6JobWidgets EXPORT KF6JobWidgetsTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

if (HAVE_QTDBUS)
    install(FILES
    dbus/org.kde.JobViewServer.xml
    DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR}
    RENAME kf6_org.kde.JobViewServer.xml)

    install(FILES
    dbus/org.kde.JobView.xml
    DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR}
    RENAME kf6_org.kde.JobView.xml)

    install(FILES
    dbus/org.kde.JobViewV2.xml
    DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR}
    RENAME kf6_org.kde.JobViewV2.xml)
endif()

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kjobwidgets_export.h
  ${KJobWidgets_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KJobWidgets COMPONENT Devel
)

ecm_qt_install_logging_categories(
    EXPORT KJOBWIDGETS
    FILE kjobwidgets.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

ecm_generate_qdoc(KF6JobWidgets kjobwidgets.qdocconf)
