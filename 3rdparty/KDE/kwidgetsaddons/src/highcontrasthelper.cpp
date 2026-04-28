/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2025 g10 Code GmbH
    SPDX-FileContributor: Ingo Klöcker <dev@ingo-kloecker.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "highcontrasthelper_p.h"

#include <QGuiApplication>
#include <QStyleHints>
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
#include <QAccessibilityHints>
#else
#ifdef Q_OS_WIN
#include <windows.h>
#endif
#endif

static bool isHighContrastModeActive()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
    return QGuiApplication::styleHints()->accessibility()->contrastPreference() == Qt::ContrastPreference::HighContrast;
#else
#ifdef Q_OS_WIN
    HIGHCONTRAST result;
    result.cbSize = sizeof(HIGHCONTRAST);
    if (SystemParametersInfo(SPI_GETHIGHCONTRAST, result.cbSize, &result, 0)) {
        return (result.dwFlags & HCF_HIGHCONTRASTON);
    }
#endif
    return false;
#endif
}

static bool isDefaultColorSchemeInUse()
{
    const QVariant colorSchemePathProperty = qApp->property("KDE_COLOR_SCHEME_PATH");
    return !colorSchemePathProperty.isValid() || colorSchemePathProperty.toString().isEmpty();
}

bool isHighContrastColorSchemeInUse()
{
    return isHighContrastModeActive() && isDefaultColorSchemeInUse();
}
