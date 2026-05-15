/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#pragma once

#include "../src/private/decorationsettingsprivate.h"

class MockSettings : public KDecoration3::DecorationSettingsPrivateV2
{
public:
    explicit MockSettings(KDecoration3::DecorationSettings *parent);

    KDecoration3::BorderSize borderSize() const override;
    QList<KDecoration3::DecorationButtonType> decorationButtonsLeft() const override;
    QList<KDecoration3::DecorationButtonType> decorationButtonsRight() const override;
    bool isAlphaChannelSupported() const override;
    bool isCloseOnDoubleClickOnMenu() const override;
    bool isOnAllDesktopsAvailable() const override;
    bool isAlwaysShowExcludeFromCapture() const override;

    void setOnAllDesktopsAvailabe(bool set);
    void setCloseOnDoubleClickOnMenu(bool set);
    void setAlwaysShowExcludeFromCapture(bool set);

private:
    bool m_onAllDesktopsAvailable = false;
    bool m_closeDoubleClickOnMenu = false;
    bool m_alwaysShowExcludeFromCapture = false;
};
