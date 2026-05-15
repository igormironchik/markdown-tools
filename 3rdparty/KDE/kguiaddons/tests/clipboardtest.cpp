/*
    SPDX-FileCopyrightText: 2025 David Edmundson <davidedmundson@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KSystemClipboard>
#include <QApplication>
#include <QClipboard>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMimeData>
#include <QPushButton>
#include <QTableView>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

// This tests KSystemClipboard
// it also shows the Qt Clipboard so that we can confirm nothing deadlocks
// copying between the two

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle(QStringLiteral("Clipboard Test"));

    auto *rootLayout = new QHBoxLayout(&window);

    // regular clipboard
    {
        auto clipboard = QApplication::clipboard();
        auto *group = new QGroupBox(QStringLiteral("Normal Clipboard"), &window);
        rootLayout->addWidget(group);

        auto *groupLayout = new QVBoxLayout(group);

        auto *clipboardView = new QTextEdit(&window);
        clipboardView->setEnabled(false);

        clipboardView->setMinimumHeight(200);
        QObject::connect(clipboard, &QClipboard::changed, clipboardView, [clipboardView, clipboard](QClipboard::Mode mode) {
            if (mode != QClipboard::Clipboard) {
                return;
            }
            clipboardView->setText(clipboard->text());
        });
        clipboardView->setText(clipboard->text());

        groupLayout->addWidget(clipboardView);

        auto *textEdit = new QTextEdit(group);
        textEdit->setPlaceholderText(QStringLiteral("Enter clipboard content here…"));
        textEdit->setMinimumHeight(120);
        groupLayout->addWidget(textEdit);

        auto *btnRow = new QHBoxLayout();
        btnRow->addStretch();
        auto *submitBtn = new QPushButton(QStringLiteral("Submit"), group);
        btnRow->addWidget(submitBtn);
        groupLayout->addLayout(btnRow);

        QObject::connect(submitBtn, &QPushButton::clicked, &window, [textEdit, clipboard]() {
            auto mimeData = new QMimeData;
            mimeData->setText(textEdit->toPlainText());
            clipboard->setMimeData(mimeData, QClipboard::Clipboard);
        });
    }
    // system clipboard
    {
        auto clipboard = KSystemClipboard::instance();
        auto *group = new QGroupBox(QStringLiteral("System Clipboard"), &window);
        rootLayout->addWidget(group);

        auto *groupLayout = new QVBoxLayout(group);

        auto *clipboardView = new QTextEdit(&window);
        clipboardView->setEnabled(false);

        clipboardView->setMinimumHeight(200);
        QObject::connect(clipboard, &KSystemClipboard::changed, clipboardView, [clipboardView, clipboard](QClipboard::Mode mode) {
            if (mode != QClipboard::Clipboard) {
                return;
            }
            clipboardView->setText(clipboard->text(mode));
        });
        clipboardView->setText(clipboard->text(QClipboard::Clipboard));

        groupLayout->addWidget(clipboardView);

        auto *textEdit = new QTextEdit(group);
        textEdit->setPlaceholderText(QStringLiteral("Enter clipboard content here…"));
        textEdit->setMinimumHeight(120);
        groupLayout->addWidget(textEdit);

        auto *btnRow = new QHBoxLayout();
        btnRow->addStretch();
        auto *submitBtn = new QPushButton(QStringLiteral("Submit"), group);
        btnRow->addWidget(submitBtn);
        groupLayout->addLayout(btnRow);

        QObject::connect(submitBtn, &QPushButton::clicked, &window, [textEdit, clipboard]() {
            auto mimeData = new QMimeData;
            mimeData->setText(textEdit->toPlainText());
            clipboard->setMimeData(mimeData, QClipboard::Clipboard);
        });
    }

    window.resize(600, 500);
    window.show();
    return app.exec();
}
