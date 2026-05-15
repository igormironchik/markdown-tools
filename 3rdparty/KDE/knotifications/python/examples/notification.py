# SPDX-FileCopyrightText: 2024 Nicolas Fella <nicolas.fella@gmx.de>
# SPDX-License-Identifier: BSD-2-Clause

import sys

from PySide6 import QtCore
from PySide6 import QtWidgets

from KNotifications import KNotification

class KNotifcationsDemo(QtWidgets.QWidget):
    def __init__(self):
        super().__init__()

        self.layout = QtWidgets.QVBoxLayout(self)

        self.button = QtWidgets.QPushButton()
        self.button.setText("Push me")
        self.button.clicked.connect(self.magic)

        self.layout.addWidget(self.button)

    @QtCore.Slot()
    def magic(self):
        noti = KNotification("notification")
        noti.setComponentName("plasma_workspace")
        noti.setTitle("Hi")
        noti.setText("Hello World")
        noti.sendEvent()

if __name__ == "__main__":
    app = QtWidgets.QApplication([])

    widget = KNotifcationsDemo()
    widget.show()

    sys.exit(app.exec())
