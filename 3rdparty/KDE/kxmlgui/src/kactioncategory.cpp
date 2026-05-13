/*
    SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kactioncategory.h"

#include <QAction>

struct KActionCategoryPrivate {
    KActionCategoryPrivate(KActionCategory *host);

    //! Our host
    KActionCategory *q;

    //! The text for this category
    QString text;

    //! List of actions
    QList<QAction *> actions;

}; // class KActionCategoryPrivate

KActionCategory::KActionCategory(const QString &text, KActionCollection *parent)
    : QObject(parent)
    , d(new KActionCategoryPrivate(this))
{
    d->text = text;
}

KActionCategory::~KActionCategory()
{
    delete d;
}

const QList<QAction *> KActionCategory::actions() const
{
    return d->actions;
}

QAction *KActionCategory::addAction(const QString &name, QAction *action)
{
    collection()->addAction(name, action);
    addAction(action);
    return action;
}

#if KXMLGUI_BUILD_DEPRECATED_SINCE(6, 9)
QAction *KActionCategory::addAction(KStandardAction::StandardAction actionType, const QObject *receiver, const char *member)
{
    QAction *action = collection()->addAction(actionType, receiver, member);
    addAction(action);
    return action;
}
#endif

#if KXMLGUI_BUILD_DEPRECATED_SINCE(6, 9)
QAction *KActionCategory::addAction(KStandardAction::StandardAction actionType, const QString &name, const QObject *receiver, const char *member)
{
    QAction *action = collection()->addAction(actionType, name, receiver, member);
    addAction(action);
    return action;
}
#endif

#if KXMLGUI_ENABLE_DEPRECATED_SINCE(6, 9)
QAction *KActionCategory::addAction(const QString &name, const QObject *receiver, const char *member)
{
    QAction *action = collection()->addAction(name, receiver, member);
    addAction(action);
    return action;
}
#endif

QAction *KActionCategory::addAction(KStandardActions::StandardAction actionType)
{
    QAction *action = collection()->addAction(actionType);
    addAction(action);
    return action;
}

QAction *KActionCategory::addAction(KStandardActions::StandardAction actionType, const QString &name)
{
    QAction *action = collection()->addAction(actionType, name);
    addAction(action);
    return action;
}

void KActionCategory::addAction(QAction *action)
{
    // Only add the action if wasn't added earlier.
    if (!d->actions.contains(action)) {
        d->actions.append(action);
    }
}

KActionCollection *KActionCategory::collection() const
{
    return qobject_cast<KActionCollection *>(parent());
}

QString KActionCategory::text() const
{
    return d->text;
}

void KActionCategory::setText(const QString &text)
{
    d->text = text;
}

void KActionCategory::unlistAction(QAction *action)
{
    // ATTENTION:
    //   This method is called from KActionCollection with an QObject formerly
    //   known as a QAction during _k_actionDestroyed(). So don't do fancy stuff
    //   here that needs a real QAction!
    d->actions.erase(std::remove(d->actions.begin(), d->actions.end(), action), d->actions.end());
}

KActionCategoryPrivate::KActionCategoryPrivate(KActionCategory *host)
    : q(host)
{
}

#include "moc_kactioncategory.cpp"
