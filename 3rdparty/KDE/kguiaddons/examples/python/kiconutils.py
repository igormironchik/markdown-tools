#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2025 Nicolas Fella <nicolas.fella@gmx.de>
# SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

import sys
from PySide6 import QtCore, QtWidgets, QtGui
from KGuiAddons import KIconUtils

class MyWidget(QtWidgets.QWidget):
    def __init__(self):
        super().__init__()

        self.icon = KIconUtils.addOverlays(QtGui.QIcon.fromTheme("cuttlefish"), ["edit-entry", "edit-delete"])

        self.label = QtWidgets.QLabel()
        self.label.setPixmap(self.icon.pixmap(64))

        self.layout = QtWidgets.QVBoxLayout(self)
        self.layout.addWidget(self.label)

if __name__ == "__main__":
    app = QtWidgets.QApplication([])

    widget = MyWidget()
    widget.resize(800, 600)
    widget.show()

    sys.exit(app.exec())
