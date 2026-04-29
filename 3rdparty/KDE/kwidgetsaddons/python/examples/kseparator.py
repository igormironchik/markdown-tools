# SPDX-FileCopyrightText: 2024 Manuel Alcaraz Zambrano <manuelalcarazzam@gmail.com>
# SPDX-License-Identifier: BSD-2-Clause

import sys

from KWidgetsAddons import KSeparator
from PySide6 import QtCore, QtWidgets


class KSeparatorDemo(QtWidgets.QWidget):
    def __init__(self):
        super().__init__()

        self.text = QtWidgets.QLabel("Hey there's a separator below me", alignment=QtCore.Qt.AlignCenter)
        self.separator = KSeparator()
        self.text2 = QtWidgets.QLabel("I'm below the separator", alignment=QtCore.Qt.AlignCenter)

        self.layout = QtWidgets.QVBoxLayout(self)
        self.layout.addWidget(self.text)
        self.layout.addWidget(self.separator)
        self.layout.addWidget(self.text2)


if __name__ == "__main__":
    app = QtWidgets.QApplication([])

    widget = KSeparatorDemo()
    widget.resize(500, 200)
    widget.show()

    sys.exit(app.exec())
