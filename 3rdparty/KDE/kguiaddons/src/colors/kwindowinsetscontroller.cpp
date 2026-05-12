// SPDX-FileCopyrightText: 2024 Mathis Brüchert <mbb@kaidan.im>
// SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "kwindowinsetscontroller.h"

#include <QCoreApplication>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#endif

class KWindowInsetsControllerPrivate
{
public:
    QColor m_statusBarColor;
    QColor m_navigationBarColor;
};

KWindowInsetsController::KWindowInsetsController(QObject *parent)
    : QObject(parent)
#ifdef Q_OS_ANDROID
    , d(new KWindowInsetsControllerPrivate)
#endif
{
}

KWindowInsetsController::~KWindowInsetsController() = default;

QColor KWindowInsetsController::statusBarBackgroundColor() const // NOLINT readability-convert-member-functions-to-static
{
#ifdef Q_OS_ANDROID
    return d->m_statusBarColor;
#else
    return {};
#endif
}

void KWindowInsetsController::setStatusBarBackgroundColor(const QColor &color) // NOLINT readability-convert-member-functions-to-static
{
#ifdef Q_OS_ANDROID
    d->m_statusBarColor = color;

    QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {
        QJniObject::callStaticMethod<void>("org.kde.guiaddons.KWindowInsetsController",
                                           "setStatusBarBackground",
                                           "(Landroid/app/Activity;I)V",
                                           QNativeInterface::QAndroidApplication::context(),
                                           color.rgba());
    });
#else
    Q_UNUSED(color)
#endif
}

QColor KWindowInsetsController::navigationBarBackgroundColor() const // NOLINT readability-convert-member-functions-to-static
{
#ifdef Q_OS_ANDROID
    return d->m_navigationBarColor;
#else
    return {};
#endif
}

void KWindowInsetsController::setNavigationBarBackgroundColor(const QColor &color) // NOLINT readability-convert-member-functions-to-static
{
#ifdef Q_OS_ANDROID
    d->m_navigationBarColor = color;

    QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {
        QJniObject::callStaticMethod<void>("org.kde.guiaddons.KWindowInsetsController",
                                           "setNavigationBarBackground",
                                           "(Landroid/app/Activity;I)V",
                                           QNativeInterface::QAndroidApplication::context(),
                                           color.rgba());
    });
#else
    Q_UNUSED(color)
#endif
}

#include "moc_kwindowinsetscontroller.cpp"
