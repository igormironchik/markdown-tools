/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2024 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <kwindowsystem.h>

#include <QApplication>
#include <QLayout>
#include <QMainWindow>
#include <QPushButton>

#include <kwaylandextras.h>

class MainWindow : public QMainWindow
{
public:
    MainWindow()
        : QMainWindow()
    {
        QMainWindow *otherWindow = new QMainWindow(this);
        otherWindow->show();
        otherWindow->setWindowState(Qt::WindowMinimized);

        QPushButton *pushButton = new QPushButton(otherWindow);
        pushButton->setText("Raise other");
        layout()->addWidget(pushButton);

        connect(pushButton, &QPushButton::clicked, this, [this, otherWindow] {
            KWaylandExtras::xdgActivationToken(windowHandle(), KWaylandExtras::lastInputSerial(windowHandle()), QString())
                .then(otherWindow, [otherWindow](const QString &token) {
                    KWindowSystem::setCurrentXdgActivationToken(token);
                    KWindowSystem::activateWindow(otherWindow->windowHandle());
                });
        });
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
