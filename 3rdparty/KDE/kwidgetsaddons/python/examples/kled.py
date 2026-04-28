# SPDX-FileCopyrightText: 2024 Manuel Alcaraz Zambrano <manuelalcarazzam@gmail.com>
# SPDX-License-Identifier: BSD-2-Clause

import sys

from KWidgetsAddons import KLed
from PySide6 import QtWidgets


class KLedDemo(QtWidgets.QWidget):
    def __init__(self):
        super().__init__()

        self.text = QtWidgets.QLabel("KLed example")
        self.led = KLed()

        self.layout = QtWidgets.QVBoxLayout(self)
        self.layout.addWidget(self.text)
        self.layout.addWidget(self.led)


if __name__ == "__main__":
    app = QtWidgets.QApplication([])

    widget = KLedDemo()
    widget.show()

    sys.exit(app.exec())
