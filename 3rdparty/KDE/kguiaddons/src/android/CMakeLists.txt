# SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

gradle_add_aar(kguiaddons_aar BUILDFILE ${CMAKE_CURRENT_SOURCE_DIR}/build.gradle NAME KF6GuiAddons)
gradle_install_aar(kguiaddons_aar DESTINATION jar)

install(
    FILES KF6GuiAddons-android-dependencies.xml
    DESTINATION ${KDE_INSTALL_LIBDIR}
    RENAME KF6GuiAddons_${CMAKE_ANDROID_ARCH_ABI}-android-dependencies.xml
)
