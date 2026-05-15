/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#pragma once

#include "../src/private/decorationbridge.h"
#include <QObject>

class MockWindow;
class MockSettings;

class MockBridge : public KDecoration3::DecorationBridge
{
    Q_OBJECT
public:
    std::unique_ptr<KDecoration3::DecoratedWindowPrivate> createClient(KDecoration3::DecoratedWindow *client, KDecoration3::Decoration *decoration) override;
    std::unique_ptr<KDecoration3::DecorationSettingsPrivate> settings(KDecoration3::DecorationSettings *parent) override;

    MockWindow *lastCreatedWindow() const
    {
        return m_lastCreatedWindow;
    }
    MockSettings *lastCreatedSettings() const
    {
        return m_lastCreatedSettings;
    }

private:
    MockWindow *m_lastCreatedWindow = nullptr;
    MockSettings *m_lastCreatedSettings = nullptr;
};
