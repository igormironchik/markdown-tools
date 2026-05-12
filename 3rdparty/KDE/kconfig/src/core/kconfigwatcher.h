/*
    SPDX-FileCopyrightText: 2018 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONFIGWATCHER_H
#define KCONFIGWATCHER_H

#include <QObject>
#include <QSharedPointer>

#include <KConfigGroup>
#include <KSharedConfig>

#include <kconfigcore_export.h>

class KConfigWatcherPrivate;

/*!
 * \class KConfigWatcher
 * \inmodule KConfigCore
 *
 * \brief Watches for configuration changes.
 *
 * The configChanged(const KConfigGroup &group, const QByteArrayList &names) signal is emitted whenever another client has updated this config file with the KConfigBase::Notify flag.
 *
 * \code
 * KConfigWatcher::Ptr watcher = KConfigWatcher::create(config);
 * QObject::connect(watcher.data(), &KConfigWatcher::configChanged, [=](){
 *     qInfo() << "Some config changed!";
 * });
 * // ...
 * generalGroup.writeEntry("SomeKey", "SomeValue", KConfigBase::Notify);
 * generalGroup.config()->reparseConfiguration();
 * \endcode
 *
 * \since 5.51
 * \sa KSharedConfig::Ptr
 * \sa KConfigBase::WriteConfigFlags
 * \sa KConfigSkeletonItem::setWriteFlags()
 * \sa KConfigGroup
 */
class KCONFIGCORE_EXPORT KConfigWatcher : public QObject
{
    Q_OBJECT
public:
    /*!
     * \typedef KConfigWatcher::Ptr
     */
    typedef QSharedPointer<KConfigWatcher> Ptr;

    /*!
     * Instantiate a ConfigWatcher for a given \a config
     *
     * \note Any additional config sources should be set before this point.
     */
    static Ptr create(const KSharedConfig::Ptr &config);

    ~KConfigWatcher() override;

    /*!
     * Returns the config being watched.
     * \since 5.66
     */
    KSharedConfig::Ptr config() const;

Q_SIGNALS:
    /*!
     * \brief Emitted when a config \a group has changed, passing the list of \a names that have changed within that group.
     *
     * The config will be reloaded before this signal is emitted.
     */
    void configChanged(const KConfigGroup &group, const QByteArrayList &names);

private Q_SLOTS:
    KCONFIGCORE_NO_EXPORT void onConfigChangeNotification(const QHash<QString, QByteArrayList> &changes);

private:
    KCONFIGCORE_NO_EXPORT explicit KConfigWatcher(const KSharedConfig::Ptr &config);
    Q_DISABLE_COPY(KConfigWatcher)
    const QScopedPointer<KConfigWatcherPrivate> d;
};

#endif
