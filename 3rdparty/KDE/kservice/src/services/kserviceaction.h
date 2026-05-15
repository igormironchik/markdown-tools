/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSERVICEACTION_H
#define KSERVICEACTION_H

#include <QSharedDataPointer>
#include <QVariant>
#include <kservice_export.h>
#include <kserviceconversioncheck_p.h>
class QVariant;
class KServiceActionPrivate;
class KService;

// we can't include kservice.h, it includes this header...
typedef QExplicitlySharedDataPointer<KService> KServicePtr;

/*!
 * \class KServiceAction
 * \inmodule KService
 *
 * \brief Represents an action in a .desktop file.
 *
 * Actions are defined with the config key Actions in the [Desktop Entry]
 * group, followed by one group per action, as per the desktop entry standard.
 * \sa KService::actions
 */
class KSERVICE_EXPORT KServiceAction
{
public:
    /*!
     * Creates a KServiceAction.
     *
     * Normally you don't have to do this, KService creates the actions
     * when parsing the .desktop file.
     *
     * \since 5.69
     */
    KServiceAction(const QString &name, const QString &text, const QString &icon, const QString &exec, bool noDisplay, const KServicePtr &service);
    /*!
     * \internal
     * Needed for operator>>
     */
    KServiceAction();

    ~KServiceAction();

    KServiceAction(const KServiceAction &other);

    KServiceAction &operator=(const KServiceAction &other);

    /*!
     * Sets the action's internal data to the given \a userData.
     */
    void setData(const QVariant &userData);

    /*!
     * Returns the action's internal data.
     */
    QVariant data() const;

    /*!
     * Returns the action's internal name
     *
     * For instance Actions=Setup;... and the group [Desktop Action Setup]
     * define an action with the name "Setup".
     */
    QString name() const;

    /*!
     * Returns the action's text, as defined by the Name key in the desktop action group
     */
    QString text() const;

    /*!
     * Returns the action's icon, as defined by the Icon key in the desktop action group
     */
    QString icon() const;

    /*!
     * Returns the action's exec command, as defined by the Exec key in the desktop action group
     */
    QString exec() const;

    /*!
     * Returns whether the action should be suppressed in menus.
     *
     * This is useful for having actions with a known name that the code
     * looks for explicitly, like Setup and Root for kscreensaver actions,
     * and which should not appear in popup menus.
     */
    bool noDisplay() const;

    /*!
     * Returns whether the action is a separator.
     *
     * This is true when the Actions key contains "_SEPARATOR_".
     */
    bool isSeparator() const;

    /*!
     * Returns the service that this action comes from
     * \since 5.69
     */
    KServicePtr service() const;

    /*!
     * Returns the requested property.
     *
     * T the type of the requested property
     *
     * \a name the name of the requested property
     *
     * \since 6.0
     */
    template<typename T>
    T property(const QString &name) const
    {
        KServiceConversionCheck::to_QVariant<T>();
        return property(name, static_cast<QMetaType::Type>(qMetaTypeId<T>())).value<T>();
    }

private:
    QSharedDataPointer<KServiceActionPrivate> d;
    friend KSERVICE_EXPORT QDataStream &operator>>(QDataStream &str, KServiceAction &act);
    friend KSERVICE_EXPORT QDataStream &operator<<(QDataStream &str, const KServiceAction &act);
    friend class KService;
    KSERVICE_NO_EXPORT void setService(const KServicePtr &service);

    QVariant property(const QString &_name, QMetaType::Type type) const;
};

KSERVICE_EXPORT QDataStream &operator>>(QDataStream &str, KServiceAction &act);
KSERVICE_EXPORT QDataStream &operator<<(QDataStream &str, const KServiceAction &act);

#endif /* KSERVICEACTION_H */
