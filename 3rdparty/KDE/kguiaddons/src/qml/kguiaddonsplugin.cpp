/*
    SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcolorutilssingleton.h"

#include <QQmlEngine>
#include <QQmlExtensionPlugin>

class KGuiAddonsPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override
    {
        qmlRegisterSingletonType(uri, 1, 0, "ColorUtils", [](QQmlEngine *engine, QJSEngine *) -> QJSValue {
            return engine->toScriptValue(KColorUtilsSingleton());
        });
    }
};

#include "kguiaddonsplugin.moc"
