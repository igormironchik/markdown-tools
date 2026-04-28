# SPDX-FileCopyrightText: 2024 Manuel Alcaraz Zambrano <manuelalcarazzam@gmail.com>
# SPDX-License-Identifier: BSD-2-Clause

import sys

from KWidgetsAddons import KPasswordDialog
from PySide6 import QtWidgets


class KPasswordDialogDemo(QtWidgets.QWidget):
    def __init__(self):
        super().__init__()

        self.text = QtWidgets.QLabel("KPasswordDialog example")

        self.button = QtWidgets.QPushButton("Change password")
        self.button.clicked.connect(self.open_dialog)

        self.layout = QtWidgets.QVBoxLayout(self)
        self.layout.addWidget(self.text)
        self.layout.addWidget(self.button)
    
    def open_dialog(self):
        self.dialog = KPasswordDialog(self)
        self.dialog.setPrompt("Enter a password")
        self.dialog.gotPassword.connect(lambda password: self.text.setText(f"Your password is: {password}"))
        self.dialog.show()


if __name__ == "__main__":
    app = QtWidgets.QApplication([])

    widget = KPasswordDialogDemo()
    widget.show()

    sys.exit(app.exec())
