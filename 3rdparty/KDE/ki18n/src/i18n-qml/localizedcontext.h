// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2025 Harald Sitter <sitter@kde.org>

#pragma once

#include <QObject>
#include <QtQmlIntegration>

#include "klocalizedqmlcontext.h"

class QJSEngine;

/*!
    \internal
    Assistive class to expose KLocalizedQmlContext as a QML singleton.
    This works around the fact that singleton construction prefers constructors over
    create() functions. We do however need to use the create() because we aren't
    really a singleton, but an engine-wide-singleton and need to excavate our actual
    context object from the engine.
*/
class LocalizedContext : public QObject
{
    Q_OBJECT
    QML_SINGLETON
    QML_FOREIGN(KLocalizedQmlContext)
    QML_NAMED_ELEMENT(KI18n)
public:
    static KLocalizedQmlContext *create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);
    LocalizedContext() = delete; // use the create() function
};
