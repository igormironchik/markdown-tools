# SPDX-FileCopyrightText: 2024 Manuel Alcaraz Zambrano <manuelalcarazzam@gmail.com>
# SPDX-License-Identifier: BSD-2-Clause

import sys

from KWidgetsAddons import KMessageWidget
from PySide6 import QtCore, QtWidgets


class KMessageWidgetDemo(QtWidgets.QWidget):
    def __init__(self):
        super().__init__()

        self.text = QtWidgets.QLabel("KMessageWidget demo", alignment=QtCore.Qt.AlignCenter)
        
        self.message_type = QtWidgets.QComboBox()
        self.message_type.addItems(["Positive", "Information", "Warning", "Error"])

        self.button = QtWidgets.QPushButton("Show message")
        self.button.clicked.connect(self.show_message)

        self.layout = QtWidgets.QVBoxLayout(self)
        self.layout.addWidget(self.text)
        self.layout.addWidget(self.message_type)
        self.layout.addWidget(self.button)

    def show_message(self):
        self.message = KMessageWidget("Be carefull!")

        match self.message_type.currentText():
            case "Positive":
                self.message.setMessageType(KMessageWidget.MessageType.Positive)
            case "Information":
                self.message.setMessageType(KMessageWidget.MessageType.Information)
            case "Warning":
                self.message.setMessageType(KMessageWidget.MessageType.Warning)
            case "Error":
                self.message.setMessageType(KMessageWidget.MessageType.Error)
        
        self.layout.insertWidget(0, self.message)


if __name__ == "__main__":
    app = QtWidgets.QApplication([])

    widget = KMessageWidgetDemo()
    widget.resize(500, 500)
    widget.show()

    sys.exit(app.exec())
