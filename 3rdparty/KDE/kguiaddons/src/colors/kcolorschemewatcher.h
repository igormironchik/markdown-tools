/*
 * SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KCOLORSCHEMEWATCHER_H
#define KCOLORSCHEMEWATCHER_H

#include "kguiaddons_export.h"

#include <QObject>

#include <memory>

class KColorSchemeWatcherPrivate;

#if KGUIADDONS_ENABLE_DEPRECATED_SINCE(6, 22)

/*!
 * \class KColorSchemeWatcher
 * \inmodule KGuiAddons
 * \brief Information about system-wide color preferences.
 * \since 5.100
 * \deprecated[6.22]
 * Use QStyleHints and QAccessibilityHints instead
 */
class KGUIADDONS_EXPORT KColorSchemeWatcher : public QObject
{
    Q_OBJECT
public:
    /*!
     * Encodes the color preference of the user to be used by applications
     * as configured in the system settings. On some systems not all values
     * are returned, e.g. PreferHighContrast is currently only returned
     * on Windows.
     *
     * \value NoPreference No preference available
     * \value PreferDark The user prefers a dark color scheme
     * \value PreferLight The user prefers a light color scheme
     * \value[since 6.13] PreferHighContrast The user prefers a system-provided high-contrast color scheme
     *
     * \sa systemPreference
     */
    enum ColorPreference {
        NoPreference = 0,
        PreferDark,
        PreferLight,
        PreferHighContrast,
    };
    Q_ENUM(ColorPreference)

    /*!
     *
     */
    KGUIADDONS_DEPRECATED_VERSION(6, 22, "Use Qt API instead")
    KColorSchemeWatcher(QObject *parent = nullptr);
    ~KColorSchemeWatcher() override;

    /*!
     * The system-wide color preference.
     */
    ColorPreference systemPreference() const;

Q_SIGNALS:
    /*!
     * Emitted when systemPreference changes.
     */
    void systemPreferenceChanged();

private:
    std::unique_ptr<KColorSchemeWatcherPrivate> const d;
};

#endif

#endif
