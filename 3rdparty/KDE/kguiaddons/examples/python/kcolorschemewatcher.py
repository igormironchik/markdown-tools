#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2025 Nicolas Fella <nicolas.fella@gmx.de>
# SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

import sys

from KGuiAddons import KColorSchemeWatcher

from PySide6 import QtCore

def print_preference(preference):
    if preference == KColorSchemeWatcher.NoPreference:
        print("No preference")
    if preference == KColorSchemeWatcher.PreferDark:
        print("Dark")
    if preference == KColorSchemeWatcher.PreferLight:
        print("Light")
    if preference == KColorSchemeWatcher.PreferHighContrast:
        print("High Contrast")


app = QtCore.QCoreApplication()

watcher = KColorSchemeWatcher()

print_preference(watcher.systemPreference())

watcher.systemPreferenceChanged.connect(lambda: print_preference(watcher.systemPreference()))

sys.exit(app.exec())
