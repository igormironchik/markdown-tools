#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2025 Nicolas Fella <nicolas.fella@gmx.de>
# SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

import sys
from PySide6 import QtCore, QtWidgets, QtGui
from KGuiAddons import KDateValidator

class MyWidget(QtWidgets.QWidget):
    def __init__(self):
        super().__init__()

        self.input = QtWidgets.QLineEdit()
        self.input.editingFinished.connect(self.onEditFinished)

        validator = KDateValidator()
        self.input.setValidator(validator)

        self.layout = QtWidgets.QVBoxLayout(self)
        self.layout.addWidget(self.input)

    @QtCore.Slot()
    def onEditFinished(self):
        print("Got valid input")

if __name__ == "__main__":
    app = QtWidgets.QApplication([])

    widget = MyWidget()
    widget.resize(800, 600)
    widget.show()

    sys.exit(app.exec())
