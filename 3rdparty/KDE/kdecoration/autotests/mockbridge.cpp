/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#include "mockbridge.h"
#include "mocksettings.h"
#include "mockwindow.h"
#include <QtGlobal>

std::unique_ptr<KDecoration3::DecoratedWindowPrivate> MockBridge::createClient(KDecoration3::DecoratedWindow *client, KDecoration3::Decoration *decoration)
{
    auto ptr = std::make_unique<MockWindow>(client, decoration);
    m_lastCreatedWindow = ptr.get();
    return ptr;
}

std::unique_ptr<KDecoration3::DecorationSettingsPrivate> MockBridge::settings(KDecoration3::DecorationSettings *parent)
{
    auto ptr = std::make_unique<MockSettings>(parent);
    m_lastCreatedSettings = ptr.get();
    return ptr;
}

#include "moc_mockbridge.cpp"
