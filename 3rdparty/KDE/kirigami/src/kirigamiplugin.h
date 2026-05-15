/*
 *  SPDX-FileCopyrightText: 2009 Alan Alpert <alan.alpert@nokia.com>
 *  SPDX-FileCopyrightText: 2010 Ménard Alexis <menard@kde.org>
 *  SPDX-FileCopyrightText: 2010 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIRIGAMIPLUGIN_H
#define KIRIGAMIPLUGIN_H

#include <QQmlEngine>
#include <QQmlExtensionPlugin>
#include <QUrl>

class KirigamiPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    KirigamiPlugin(QObject *parent = nullptr);
    void registerTypes(const char *uri) override;

#ifdef KIRIGAMI_BUILD_TYPE_STATIC
    static KirigamiPlugin &getInstance();
    static void registerTypes(QQmlEngine *engine = nullptr);
#endif

Q_SIGNALS:
    void languageChangeEvent();
};

#endif
