set(backend_sources
    udisksmanager.cpp
    udisksdevice.cpp
    udisksblock.cpp
    udisksstoragevolume.cpp
    udisksdeviceinterface.cpp
    udisksopticaldisc.cpp
    udisksopticaldrive.cpp
    udisksstoragedrive.cpp
    udisksstorageaccess.cpp
    udisksgenericinterface.cpp
    udisksutils.cpp
    dbus/manager.cpp
)

ecm_qt_declare_logging_category(backend_sources
    HEADER udisks_debug.h
    IDENTIFIER Solid::Backends::UDisks2::UDISKS2
    DEFAULT_SEVERITY Warning
    CATEGORY_NAME kf.solid.backends.udisks2
    OLD_CATEGORY_NAMES org.kde.solid.udisks2
    DESCRIPTION "UDisks2 (Solid)"
    EXPORT SOLID
)
