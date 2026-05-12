/*
    SPDX-FileCopyrightText: 2005-2009 Olivier Goffart <ogoffart at kde.org>
    SPDX-FileCopyrightText: 2023 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KNOTIFYCONFIG_H
#define KNOTIFYCONFIG_H

#include "knotifications_export.h"
#include <QSharedDataPointer>

class KNotifyConfigPrivate;

/*!
 * \class KNotifyConfig
 * \inmodule KNotifications
 *
 * \brief Represent the configuration for an event.
 */
class KNOTIFICATIONS_EXPORT KNotifyConfig
{
public:
    /*!
     * Creates a notify config for the given application name and event id
     *
     * \a applicationName The application name, typically the name of the notifyrc file without its extension.
     *
     * \a eventId The notification event ID, i.e. the part after Event/ in its notifyrc file.
     */
    KNotifyConfig(const QString &applicationName, const QString &eventId);
    ~KNotifyConfig();

    KNotifyConfig(const KNotifyConfig &other);
    KNotifyConfig &operator=(const KNotifyConfig &other);

    /*!
     * the name of the application that triggered the notification
     */
    QString applicationName() const;

    /*!
     * the name of the notification
     */
    QString eventId() const;

    /*!
     * Whether there exists an event with the given id under the given application name.
     */
    bool isValid() const;

    /*!
     * Returns entry from the relevant Global notifyrc config group
     *
     * This will return the configuration from the user for the given key.
     * It first look into the user config file, and then in the global config file.
     *
     * Returns a null string if the entry doesn't exist
     */
    QString readGlobalEntry(const QString &key) const;

    /*!
     * Returns entry from the relevant Event/ notifyrc config group
     *
     * This will return the configuration from the user for the given key.
     * It first look into the user config file, and then in the global config file.
     *
     * Returns a null string if the entry doesn't exist
     */
    QString readEntry(const QString &key) const;

    /*!
     * Returns path entry from the relevant Event/ notifyrc config group
     *
     * This will return the configuration from the user for the given key
     * and interpret it as a path.
     */
    QString readPathEntry(const QString &key) const;

    /*!
     * reparse the cached configs.  to be used when the config may have changed
     */
    static void reparseConfiguration();

    /*!
     *
     */
    static void reparseSingleConfiguration(const QString &app);

private:
    QSharedDataPointer<KNotifyConfigPrivate> d;
};

#endif
