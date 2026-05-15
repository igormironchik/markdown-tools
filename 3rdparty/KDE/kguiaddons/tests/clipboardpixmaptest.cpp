/*
    SPDX-FileCopyrightText: 2026 David Edmundson <davidedmundson@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KSystemClipboard>

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QPixmap>
#include <QTimer>

// Check that we can send massive content without any timeouts

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QPixmap pixmap(8000, 8000);
    pixmap.fill(Qt::red);

    auto mimeData = new QMimeData;
    mimeData->setImageData(pixmap);

    auto clipboard = KSystemClipboard::instance();
    clipboard->setMimeData(mimeData, QClipboard::Clipboard);

    QTimer::singleShot(5000, &app, [&app]() {
        app.quit();
    });

    return app.exec();
}
