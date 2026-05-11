set(libkdecoration3Private_SRCS
    decoratedwindowprivate.cpp
    decoratedwindowprivate.h
    decorationbridge.cpp
    decorationbridge.h
    decorationsettingsprivate.cpp
    decorationsettingsprivate.h
)

add_library(kdecorations3private SHARED ${libkdecoration3Private_SRCS})

generate_export_header(
    kdecorations3private
BASE_NAME
    KDECORATIONS_PRIVATE
EXPORT_FILE_NAME
    kdecoration3/private/kdecoration3_private_export.h
)

add_library(KDecoration3::KDecorationPrivate ALIAS kdecorations3private)

target_link_libraries(kdecorations3private
    PUBLIC
        Qt::Core
        Qt::Gui
)

target_include_directories(kdecorations3private INTERFACE "$<INSTALL_INTERFACE:${KDECORATION3_INCLUDEDIR}>" )

set_target_properties(kdecorations3private PROPERTIES VERSION   ${KDECORATION3_VERSION}
                                                      SOVERSION 2
                                                      EXPORT_NAME KDecoration3Private
)

ecm_generate_headers(KDecoration3Private_CamelCase_HEADERS
  HEADER_NAMES
    DecoratedWindowPrivate
    DecorationBridge
    DecorationSettingsPrivate
  PREFIX
    KDecoration3/Private
  REQUIRED_HEADERS KDecoration3Private_HEADERS
)
install(FILES ${KDecoration3Private_CamelCase_HEADERS}
        DESTINATION ${KDECORATION3_INCLUDEDIR}/KDecoration3/Private
        COMPONENT Devel)

install(TARGETS kdecorations3private EXPORT KDecoration3Targets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/kdecoration3/private/kdecoration3_private_export.h
        ${KDecoration3Private_HEADERS}
    DESTINATION
        ${KDECORATION3_INCLUDEDIR}/kdecoration3/private
    COMPONENT
        Devel
)
