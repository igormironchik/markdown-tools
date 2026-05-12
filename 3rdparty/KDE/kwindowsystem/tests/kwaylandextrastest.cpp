/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2023 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <kwaylandextras.h>
#include <kwindowsystem.h>

#include <QApplication>
#include <QWidget>

#include "ui_kwaylandextrastest.h"

class Window : public QWidget
{
public:
    Window();

private:
    void updateSerial();
    void requestToken();

    void exportWindow();
    void unexportWindow();
    void setExportedHandle(const QString &handle);

    Ui_KWaylandExtrasTest m_ui;
};

Window::Window()
{
    m_ui.setupUi(this);

    updateSerial();

    connect(m_ui.serialButton, &QPushButton::clicked, this, &Window::updateSerial);
    connect(m_ui.tokenButton, &QPushButton::clicked, this, &Window::requestToken);
    connect(m_ui.exportButton, &QPushButton::clicked, this, &Window::exportWindow);
    connect(m_ui.unexportButton, &QPushButton::clicked, this, &Window::unexportWindow);
}

void Window::updateSerial()
{
    m_ui.serialLabel->setText(QString::number(KWaylandExtras::self()->lastInputSerial(windowHandle())));
}

void Window::requestToken()
{
    KWaylandExtras::xdgActivationToken(windowHandle(), KWaylandExtras::self()->lastInputSerial(windowHandle()), QString())
        .then(this, [this](const QString &token) {
            m_ui.tokenLabel->setText(token);
        });
}

void Window::exportWindow()
{
    connect(
        KWaylandExtras::self(),
        &KWaylandExtras::windowExported,
        this,
        [this](QWindow *window, const QString &handle) {
            Q_UNUSED(window);
            setExportedHandle(handle);
        },
        Qt::SingleShotConnection);
    KWaylandExtras::exportWindow(windowHandle());
}

void Window::unexportWindow()
{
    KWaylandExtras::unexportWindow(windowHandle());
    setExportedHandle(QString());
}

void Window::setExportedHandle(const QString &handle)
{
    m_ui.exportedLabel->setText(handle);
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    Window window;
    window.show();
    return app.exec();
}
