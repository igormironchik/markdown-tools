/*
 * SPDX-FileCopyrightText: 2020 Piyush Aggarwal <piyushaggarwal002@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kcolorschemewatcher_win.h"
#include "kcolorschemewatcher.h"

#include <windows.h>

#include <QAbstractEventDispatcher>

#include <QDebug>

static bool isHighContrastModeActive()
{
    HIGHCONTRAST result;
    result.cbSize = sizeof(HIGHCONTRAST);
    if (SystemParametersInfo(SPI_GETHIGHCONTRAST, result.cbSize, &result, 0)) {
        return (result.dwFlags & HCF_HIGHCONTRASTON);
    }
    return false;
}

KColorSchemeWatcherWin::KColorSchemeWatcherWin()
{
    QAbstractEventDispatcher::instance()->installNativeEventFilter(this);

    m_preference = getCurrentSystemPreference();
}

bool KColorSchemeWatcherWin::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *)
{
    MSG *msg = static_cast<MSG *>(message);
    switch (msg->message) {
    case WM_SETTINGCHANGE: {
        m_settings.sync();
        const auto currentPreference = getCurrentSystemPreference();
        if (m_preference != currentPreference) {
            m_preference = currentPreference;
            Q_EMIT systemPreferenceChanged();
        }

        break;
    }
    default: {
    }
    }
    return false;
}

KColorSchemeWatcher::ColorPreference KColorSchemeWatcherWin::systemPreference() const
{
    return m_preference;
}

KColorSchemeWatcher::ColorPreference KColorSchemeWatcherWin::getCurrentSystemPreference() const
{
    if (isHighContrastModeActive()) {
        return KColorSchemeWatcher::PreferHighContrast;
    } else if (m_settings.value(QStringLiteral("AppsUseLightTheme"), true).value<bool>()) {
        return KColorSchemeWatcher::PreferLight;
    }
    return KColorSchemeWatcher::PreferDark;
}

#include "moc_kcolorschemewatcher_win.cpp"
