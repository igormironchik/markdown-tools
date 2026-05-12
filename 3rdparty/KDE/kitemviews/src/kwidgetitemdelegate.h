/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007-2008 Rafael Fernández López <ereslibre@kde.org>
    SPDX-FileCopyrightText: 2008 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KWIDGETITEMDELEGATE_H
#define KWIDGETITEMDELEGATE_H

#include <QAbstractItemDelegate>
#include <QEvent>
#include <QList>
#include <QPersistentModelIndex>
#include <memory>

#include <kitemviews_export.h>

class QObject;
class QPainter;
class QStyleOption;
class QStyleOptionViewItem;
class QAbstractItemView;
class QItemSelection;

class KWidgetItemDelegatePrivate;
class KWidgetItemDelegatePool;

/*!
 * \class KWidgetItemDelegate
 * \inmodule KItemViews
 *
 * \brief This class allows to create item delegates embedding simple widgets to interact
 * with items.
 *
 * For instance you can add push buttons, line edits, etc. to your delegate
 * and use them to modify the state of your model.
 *
 * Porting from KF5 to KF6:
 *
 * The signature of the virtual method
 * KWidgetItemDelegate::updateItemWidgets(const QList<QWidget *>, const QStyleOptionViewItem &, const QPersistentModelIndex &) const
 * was changed to ;
 * KWidgetItemDelegate::updateItemWidgets(const QList<QWidget *> &, const QStyleOptionViewItem &, const QPersistentModelIndex &) const.
 *
 * \since 4.1
 */
class KITEMVIEWS_EXPORT KWidgetItemDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    /*!
     * Creates a new ItemDelegate to be used with a given itemview.
     *
     * \a itemView the item view the new delegate will monitor
     *
     * \a parent the parent of this delegate
     */
    explicit KWidgetItemDelegate(QAbstractItemView *itemView, QObject *parent = nullptr);

    ~KWidgetItemDelegate() override;

    /*!
     * Retrieves the item view this delegate is monitoring.
     */
    QAbstractItemView *itemView() const;

    /*!
     * Retrieves the currently focused index. An invalid index if none is focused.
     */
    QPersistentModelIndex focusedIndex() const;

    /*!
     * trigger a modelReset
     */
    void resetModel();

protected:
    /*!
     * Creates the list of widgets needed for an item.
     *
     * \note No initialization of the widgets is supposed to happen here.
     *       The widgets will be initialized based on needs for a given item.
     *
     * \note If you want to connect some widget signals to any slot, you should
     *       do it here.
     *
     * \a index the index to create widgets for
     *
     * Returns the list of newly created widgets which will be used to interact with an item.
     * \sa updateItemWidgets()
     */
    virtual QList<QWidget *> createItemWidgets(const QModelIndex &index) const = 0;

    /*!
     * Updates a list of widgets for its use inside of the delegate (painting or
     * event handling).
     *
     * \note All the positioning and sizing should be done in item coordinates.
     *
     * \warning Do not make widget connections in here, since this method will
     * be called very regularly.
     *
     * \a widgets the widgets to update
     *
     * \a option the current set of style options for the view.
     *
     * \a index the model index of the item currently manipulated.
     */
    virtual void updateItemWidgets(const QList<QWidget *> &widgets, const QStyleOptionViewItem &option, const QPersistentModelIndex &index) const = 0;

    /*!
     * Sets the list of event \a types that a \a widget will block.
     *
     * Blocked events are not passed to the view. This way you can prevent an item
     * from being selected when a button is clicked for instance.
     *
     * \a widget the widget which must block events
     *
     * \a types the list of event types the widget must block
     */
    void setBlockedEventTypes(QWidget *widget, const QList<QEvent::Type> &types) const;

    /*!
     * Retrieves the list of blocked event types for the given widget.
     *
     * \a widget the specified widget.
     *
     * Returns the list of blocked event types, can be empty if no events are blocked.
     */
    QList<QEvent::Type> blockedEventTypes(QWidget *widget) const;

private:
    friend class KWidgetItemDelegatePool;
    friend class KWidgetItemDelegateEventListener;
    std::unique_ptr<class KWidgetItemDelegatePrivate> const d;

    Q_PRIVATE_SLOT(d, void _k_slotRowsInserted(const QModelIndex &, int, int))
    Q_PRIVATE_SLOT(d, void _k_slotRowsAboutToBeRemoved(const QModelIndex &, int, int))
    Q_PRIVATE_SLOT(d, void _k_slotRowsRemoved(const QModelIndex &, int, int))
    Q_PRIVATE_SLOT(d, void _k_slotDataChanged(const QModelIndex &, const QModelIndex &))
    Q_PRIVATE_SLOT(d, void _k_slotLayoutChanged())
    Q_PRIVATE_SLOT(d, void _k_slotModelReset())
    Q_PRIVATE_SLOT(d, void _k_slotSelectionChanged(const QItemSelection &, const QItemSelection &))
};

#endif
