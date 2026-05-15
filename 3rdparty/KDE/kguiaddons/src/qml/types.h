// SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#ifndef KGUIADDONS_QML_TYPES
#define KGUIADDONS_QML_TYPES

#include <KWindowInsetsController>

#include <QQmlEngine>

class KWindowInsetsControllerForeign : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(WindowInsetsController)
    QML_FOREIGN(KWindowInsetsController)
    QML_SINGLETON
};

#endif
