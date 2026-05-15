#!/usr/bin/env python3
"""
SPDX-FileCopyrightText: 2025 David Edmundson <davidedmundson@kde.org>
SPDX-FileCopyrightText: 2026 Fushan Wen <qydwhotmail@gmail.com>
SPDX-License-Identifier: LGPL-2.0-or-later
"""

import sys

from KGuiAddons import KSystemClipboard
from PySide6.QtCore import QMimeData, Slot
from PySide6.QtGui import QClipboard
from PySide6.QtWidgets import (QApplication, QHBoxLayout, QLabel, QPushButton, QSizePolicy, QTextEdit, QVBoxLayout, QWidget)


class KSystemClipboardExample(QWidget):

    def __init__(self) -> None:
        super().__init__()
        self.setWindowTitle("KSystemClipboard")
        self.__setup_ui()

    def __setup_ui(self) -> None:
        self.system_clipboard_view = QTextEdit()
        self.system_text_edit = QTextEdit()
        self.__setup_system_clipboard()

        self.resize(600, 500)

    def __setup_system_clipboard(self) -> None:
        clipboard = KSystemClipboard.instance()

        layout = QVBoxLayout(self)

        title_label = QLabel("System Clipboard (KSystemClipboard)")
        layout.addWidget(title_label)

        self.system_clipboard_view.setReadOnly(True)
        self.system_clipboard_view.setMinimumHeight(200)
        self.system_clipboard_view.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)

        clipboard.changed.connect(self.on_system_clipboard_changed)

        self.system_clipboard_view.setText(clipboard.text(QClipboard.Mode.Clipboard))

        layout.addWidget(self.system_clipboard_view)

        self.system_text_edit.setPlaceholderText("Enter clipboard content here…")
        self.system_text_edit.setMinimumHeight(120)
        self.system_text_edit.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        layout.addWidget(self.system_text_edit)

        btn_row = QHBoxLayout()
        btn_row.addStretch()

        submit_btn = QPushButton("Submit")
        submit_btn.clicked.connect(self.submit_system_clipboard)
        btn_row.addWidget(submit_btn)

        layout.addLayout(btn_row)

    @Slot(QClipboard.Mode)
    def on_system_clipboard_changed(self, mode) -> None:
        if mode != QClipboard.Mode.Clipboard:
            return
        clipboard = KSystemClipboard.instance()
        self.system_clipboard_view.setText(clipboard.text(mode))

    @Slot()
    def submit_system_clipboard(self) -> None:
        clipboard = KSystemClipboard.instance()
        mime_data = QMimeData()
        mime_data.setText(self.system_text_edit.toPlainText())
        clipboard.setMimeData(mime_data, QClipboard.Mode.Clipboard)


if __name__ == "__main__":
    app = QApplication(sys.argv)

    window = KSystemClipboardExample()
    window.show()

    sys.exit(app.exec())
