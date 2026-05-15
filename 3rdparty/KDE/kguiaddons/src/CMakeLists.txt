configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_library(KF6GuiAddons)
add_library(KF6::GuiAddons ALIAS KF6GuiAddons)

set_target_properties(KF6GuiAddons PROPERTIES
    VERSION     ${KGUIADDONS_VERSION}
    SOVERSION   ${KGUIADDONS_SOVERSION}
    EXPORT_NAME GuiAddons
)

target_sources(KF6GuiAddons PRIVATE
 colors/kcolorspaces.cpp
 colors/kcolorutils.cpp
 colors/kcolorcollection.cpp
 colors/kcolormimedata.cpp
 colors/kcolorschemewatcher.cpp
 colors/kcolorschemewatcherbackend.cpp
 colors/kcolorschemewatcher_qt.cpp
 colors/kwindowinsetscontroller.cpp
 text/kdatevalidator.cpp
 text/kwordwrap.cpp
 fonts/kfontutils.cpp
 util/kiconutils.cpp
 util/klocalimagecacheimpl.cpp
 util/kmodifierkeyinfo.cpp
 util/kmodifierkeyinfoprovider.cpp
 util/kurlhandler_p.cpp
 util/kcursorsaver.cpp
 util/kcountryflagemojiiconengine.cpp
 util/kjobwindows.cpp
 recorder/keyboardgrabber.cpp
 recorder/kkeysequencerecorder.cpp
 systemclipboard/qtclipboard.cpp
 systemclipboard/ksystemclipboard.cpp

 colors/kcolorspaces_p.h
 colors/kcolorutils.h
 colors/kcolorcollection.h
 colors/kcolormimedata.h
 text/kdatevalidator.h
 text/kwordwrap.h
 fonts/kfontutils.h
 util/kiconutils.h
 util/klocalimagecacheimpl.h
 util/kmodifierkeyinfo.h
 util/kmodifierkeyinfoprovider_p.h
 util/kurlhandler_p.h
 util/kcursorsaver.h
 util/kcountryflagemojiiconengine.h
 util/kjobwindows.h
 recorder/keyboardgrabber_p.h
 recorder/kkeysequencerecorder.h
 systemclipboard/qtclipboard_p.h
 systemclipboard/ksystemclipboard.h
)

ecm_qt_declare_logging_category(KF6GuiAddons
    HEADER kguiaddons_debug.h
    IDENTIFIER KGUIADDONS_LOG
    CATEGORY_NAME kf.guiaddons
    DESCRIPTION "KGuiAddons"
    EXPORT KGUIADDONS
)

if(WIN32)
    target_sources(KF6GuiAddons PRIVATE
        colors/kcolorschemewatcher_win.cpp
        util/ksysteminhibitor_windows.cpp
    )
    target_link_libraries(KF6GuiAddons PRIVATE advapi32)
endif()

if(APPLE)
    find_library(APPKIT_LIBRARY AppKit)
    target_sources(KF6GuiAddons PRIVATE colors/kcolorschemewatcher_mac.mm)
    target_link_libraries(KF6GuiAddons PRIVATE ${APPKIT_LIBRARY})
endif()

if(WITH_DBUS)
    target_sources(KF6GuiAddons PRIVATE
        colors/kcolorschemewatcher_xdg.cpp
        util/ksysteminhibitor_dbus.cpp
    )
    target_link_libraries(KF6GuiAddons PRIVATE Qt6::DBus Qt6::GuiPrivate)
    target_compile_definitions(KF6GuiAddons PRIVATE WITH_DBUS)
endif()

if(WITH_WAYLAND)
    qt_generate_wayland_protocol_client_sources(KF6GuiAddons
        PRIVATE_CODE
        FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/recorder/keyboard-shortcuts-inhibit-unstable-v1.xml"
            "${CMAKE_CURRENT_SOURCE_DIR}/systemclipboard/wlr-data-control-unstable-v1.xml"
            "${PLASMA_WAYLAND_PROTOCOLS_DIR}/keystate.xml"
            "${WaylandProtocols_DATADIR}/staging/ext-data-control/ext-data-control-v1.xml"
            "${Wayland_DATADIR}/wayland.xml"
    )
    target_sources(KF6GuiAddons PRIVATE
        recorder/waylandinhibition.cpp
        systemclipboard/waylandclipboard.cpp
        systemclipboard/wlrwaylandclipboard.cpp
        util/kmodifierkeyinfoprovider_wayland.cpp
        recorder/waylandinhibition_p.h
        systemclipboard/waylandclipboard_p.h
        systemclipboard/waylandpipewriterhelper.cpp
        systemclipboard/wlrwaylandclipboard_p.h
        util/kmodifierkeyinfoprovider_wayland.h
    )
    target_compile_definitions(KF6GuiAddons PRIVATE WITH_WAYLAND)
    target_link_libraries(KF6GuiAddons PRIVATE Qt6::GuiPrivate Qt6::WaylandClient Wayland::Client)
endif()

if(WITH_X11)
    target_sources(KF6GuiAddons PRIVATE util/kmodifierkeyinfoprovider_xcb.cpp util/kmodifierkeyinfoprovider_xcb.h)
    target_link_libraries(KF6GuiAddons PRIVATE X11::Xkb XCB::XCB)
endif()

if(ANDROID)
    target_sources(KF6GuiAddons PRIVATE
        util/ksysteminhibitor_android.cpp
    )
    add_subdirectory(android)
endif()

if (NOT WITH_DBUS AND NOT ANDROID AND NOT WIN32)
    target_sources(KF6GuiAddons PRIVATE
        util/ksysteminhibitor_dummy.cpp
    )
endif()

ecm_generate_export_header(KF6GuiAddons
    BASE_NAME KGuiAddons
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 6.3 6.12 6.22
)

set(kguiaddons_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}/colors
  ${CMAKE_CURRENT_SOURCE_DIR}/fonts
  ${CMAKE_CURRENT_SOURCE_DIR}/text
  ${CMAKE_CURRENT_SOURCE_DIR}/util
  ${CMAKE_CURRENT_SOURCE_DIR}/recorder
  ${CMAKE_CURRENT_SOURCE_DIR}/systemclipboard
  )
target_include_directories(KF6GuiAddons PUBLIC "$<BUILD_INTERFACE:${kguiaddons_INCLUDES}>")
target_include_directories(KF6GuiAddons INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KGuiAddons>" )
target_compile_definitions(KF6GuiAddons INTERFACE "$<INSTALL_INTERFACE:KGUIADDONS_LIB>")
target_link_libraries(KF6GuiAddons PUBLIC Qt6::Gui)

qt_extract_metatypes(KF6GuiAddons)

ecm_generate_headers(KGuiAddons_HEADERS
  HEADER_NAMES
  KColorUtils
  KColorCollection
  KColorMimeData
  KColorSchemeWatcher
  KWindowInsetsController

  RELATIVE colors
  REQUIRED_HEADERS KGuiAddons_HEADERS
)
ecm_generate_headers(KGuiAddons_HEADERS
  HEADER_NAMES
  KDateValidator
  KWordWrap

  RELATIVE text
  REQUIRED_HEADERS KGuiAddons_HEADERS
)
ecm_generate_headers(KGuiAddons_HEADERS
  HEADER_NAMES
  KFontUtils

  RELATIVE fonts
  REQUIRED_HEADERS KGuiAddons_HEADERS
)
ecm_generate_headers(KGuiAddons_HEADERS
  HEADER_NAMES
  KIconUtils
  KImageCache
  KModifierKeyInfo
  KCursorSaver
  KCountryFlagEmojiIconEngine
  KJobWindows
  KSystemInhibitor

  RELATIVE util
  REQUIRED_HEADERS KGuiAddons_HEADERS
)

ecm_generate_headers(KGuiAddons_HEADERS
    HEADER_NAMES
    KKeySequenceRecorder

    RELATIVE recorder
    REQUIRED_HEADERS KGuiAddons_HEADERS
)

#add_subdirectory(systemclipboard)

ecm_generate_headers(KGuiAddons_HEADERS
    HEADER_NAMES
    KSystemClipboard

    RELATIVE systemclipboard
    REQUIRED_HEADERS KGuiAddons_HEADERS
)

install(TARGETS KF6GuiAddons EXPORT KF6GuiAddonsTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kguiaddons_export.h
  util/kmodifierkeyinfoprovider_p.h
  util/klocalimagecacheimpl.h # implementation detail, no forwarding header
  ${KGuiAddons_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KGuiAddons COMPONENT Devel
)

ecm_generate_qdoc(KF6GuiAddons kguiaddons.qdocconf)

ecm_qt_install_logging_categories(
    EXPORT KGUIADDONS
    FILE kguiaddons.categories
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)

if(BUILD_TESTING)
    add_executable(pasteclient systemclipboard/tests/paste.cpp)

    target_link_libraries(pasteclient
        KF6GuiAddons
    )
endif()

if (NOT ANDROID AND BUILD_GEO_SCHEME_HANDLER)
    add_subdirectory(geo-scheme-handler)
endif()

if (TARGET Qt6::Qml)
    add_subdirectory(qml)
endif()
