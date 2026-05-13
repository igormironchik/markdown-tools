
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/kirigami/src)
include(${CMAKE_SOURCE_DIR}/3rdparty/kirigami/KF6Kirigami2Macros.cmake)

set(minimal_SRCS
    main.cpp
    )

qt_add_resources(RESOURCES kirigami-icons.qrc resources.qrc)

if (ANDROID)
    set(minimal_EXTRA_LIBS
    #FIXME: we shouldn't have to link to it but otherwise the lib won't be packaged on Android
    Qt6::QuickControls2)
else ()
#qstyle-based qqc2 style needs a QApplication
    set(minimal_EXTRA_LIBS Qt6::Widgets)
endif()


add_executable(minimal ${minimal_SRCS} ${RESOURCES})
#kirigamiplugin is the static library built by us
target_link_libraries(minimal kirigamiplugin Qt6::Core  Qt6::Qml Qt6::Quick Qt6::QuickControls2 ${minimal_EXTRA_LIBS})

#install(TARGETS minimal ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

kirigami_package_breeze_icons(ICONS open-menu-symbolic document-decrypt folder-sync go-next go-previous go-up handle-left handle-right view-list-icons applications-graphics media-record-symbolic)
