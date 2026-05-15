/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
    SPDX-FileCopyrightText: 1997-1999 Matthias Kalle Dalheimer <kalle@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSHAREDCONFIG_H
#define KSHAREDCONFIG_H

#include <QExplicitlySharedDataPointer>
#include <kconfig.h>

/*!
 * \class KSharedConfig
 * \inmodule KConfigCore
 *
 * \brief KConfig variant using shared memory.
 *
 * KSharedConfig provides a shared (reference counted) variant
 * of KConfig. This allows you to use/manipulate the same configuration
 * files from different places in your code without worrying about
 * accidentally overwriting changes.
 *
 * The openConfig() method is threadsafe: every thread gets a separate repository
 * of shared KConfig objects. This means, however, that you'll be responsible for
 * synchronizing the instances of KConfig for the same filename between threads,
 * using KConfig::reparseConfiguration() after a manual change notification, just like you have
 * to do between processes.
 *
 * Load a specific configuration file:
 * \code
 * KSharedConfigPtr config = KSharedConfig::openConfig("/etc/kderc");
 * \endcode
 *
 * Load the configuration for an application stored in \c ~/.config/appname/appnamerc:
 * \code
 * KSharedConfigPtr config = KSharedConfig::openConfig("appnamerc", KConfig::SimpleConfig, QStandardPaths::AppConfigLocation);
 * \endcode
 * The \c appname should match the name set via QCoreApplication::setApplicationName or the component argument of KAboutData::KAboutData.
 *
 * Load the configuration for an application \c ~/.config/appnamerc:
 * \code
 * KSharedConfigPtr config = KSharedConfig::openConfig("appnamerc");
 * \endcode
 *
 * Load the user-specific data files for an application in \c ~/.local/share/appname/appnamerc:
 * \code
 * KSharedConfigPtr config = KSharedConfig::openConfig("appnamerc", KConfig::NoGlobals, QStandardPaths::AppDataLocation);
 * \endcode
 *
 * \sa KSharedConfig, KConfigGroup, {https://develop.kde.org/docs/features/configuration/introduction/}{Introduction to KConfig}
 */
class KCONFIGCORE_EXPORT KSharedConfig : public KConfig, public QSharedData // krazy:exclude=dpointer (only for refcounting)
{
public:
    /*!
     * \typedef KSharedConfig::Ptr
     *
     * A pointer to a KSharedConfig object.
     */
    typedef QExplicitlySharedDataPointer<KSharedConfig> Ptr;

public:
    /*!
     * Creates a KSharedConfig object to manipulate a configuration file.
     *
     * If an absolute path is specified for \a fileName, that file will be used
     * as the store for the configuration settings. If a non-absolute path
     * is provided, the file will be looked for in the standard directory
     * specified by \a type. If no path is provided, a default
     * configuration file will be used based on the name of the main
     * application component as defined in QCoreApplication::setApplicationName
     * or the component name of KAboutData::KAboutData, plus rc.
     * For example: "appnamerc".
     *
     * The \a mode determines whether the user or global settings will be allowed
     * to influence the values returned by this object.
     *
     * \sa KConfig, KConfig::OpenFlags, QStandardPaths
     */
    static KSharedConfig::Ptr
    openConfig(const QString &fileName = QString(), OpenFlags mode = FullConfig, QStandardPaths::StandardLocation type = QStandardPaths::GenericConfigLocation);

    /*!
     * Creates a KSharedConfig object to manipulate a configuration file suitable
     * for storing state information. Use this for storing information that is
     * changing frequently and should not be saved by configuration backup
     * utilities.
     *
     * If an absolute path is specified for \a fileName, that file will be used
     * as the store for the configuration settings. If a non-absolute path
     * is provided, the file will be looked for in the standard state directory
     * (QStandardPaths::GenericStateLocation). If no path is provided, a default
     * configuration file will be used based on the name of the main
     * application component as defined in QCoreApplication::setApplicationName
     * or the component name of KAboutData::KAboutData, plus rc.
     * For example: "appnamerc".
     *
     * \since 5.67
     *
     * \sa KConfig
     */
    static KSharedConfig::Ptr openStateConfig(const QString &fileName = QString());

    ~KSharedConfig() override;

private:
    Q_DISABLE_COPY(KSharedConfig)
    KConfigGroup groupImpl(const QString &groupName) override;
    const KConfigGroup groupImpl(const QString &groupName) const override;

    KCONFIGCORE_NO_EXPORT KSharedConfig(const QString &file, OpenFlags mode, QStandardPaths::StandardLocation resourceType);
};

/*!
 * \typedef KSharedConfigPtr
 * \relates KSharedConfig
 */
typedef KSharedConfig::Ptr KSharedConfigPtr;

#endif // multiple inclusion guard
