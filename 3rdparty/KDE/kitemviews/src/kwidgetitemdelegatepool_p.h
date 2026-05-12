/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2008 Rafael Fernández López <ereslibre@kde.org>
    SPDX-FileCopyrightText: 2008 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KWIDGETITEMDELEGATEPOOL_P_H
#define KWIDGETITEMDELEGATEPOOL_P_H

#include <QHash>
#include <QList>
#include <QPersistentModelIndex>

class QWidget;
class QStyleOptionViewItem;
class KWidgetItemDelegate;
class KWidgetItemDelegatePoolPrivate;

class KWidgetItemDelegatePool
{
public:
    enum UpdateWidgetsEnum {
        UpdateWidgets = 0,
        NotUpdateWidgets,
    };

    KWidgetItemDelegatePool(KWidgetItemDelegate *delegate);

    ~KWidgetItemDelegatePool();

    KWidgetItemDelegatePool(const KWidgetItemDelegatePool &) = delete;
    KWidgetItemDelegatePool &operator=(const KWidgetItemDelegatePool &) = delete;

    /*
     * Returns the widget associated to index and widget
     * index The index to search into.
     * option a QStyleOptionViewItem.
     * Returns a QList of the pointers to the widgets found.
     */
    QList<QWidget *> findWidgets(const QPersistentModelIndex &index, const QStyleOptionViewItem &option, UpdateWidgetsEnum updateWidgets = UpdateWidgets) const;

    QList<QWidget *> invalidIndexesWidgets() const;

    void fullClear();

private:
    friend class KWidgetItemDelegate;
    friend class KWidgetItemDelegatePrivate;
    KWidgetItemDelegatePoolPrivate *const d;
};

class KWidgetItemDelegateEventListener;

class KWidgetItemDelegatePoolPrivate
{
public:
    KWidgetItemDelegatePoolPrivate(KWidgetItemDelegate *d);

    KWidgetItemDelegate *delegate;
    KWidgetItemDelegateEventListener *eventListener;

    QHash<QPersistentModelIndex, QList<QWidget *>> usedWidgets;
    QHash<QWidget *, QPersistentModelIndex> widgetInIndex;

    bool clearing = false;
};

#endif
