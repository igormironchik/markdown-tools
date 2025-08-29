/*
    SPDX-FileCopyrightText: 2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QStyledItemDelegate>
#include <QTreeView>

QT_BEGIN_NAMESPACE
class QSortFilterProxyModel;
QT_END_NAMESPACE

namespace MdEditor
{

class TocModel;

//! Item delegate for word wrapping.
class WordWrapItemDelegate final : public QStyledItemDelegate
{
public:
    WordWrapItemDelegate(QTreeView *parent,
                         TocModel *model,
                         QSortFilterProxyModel *sortModel);

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

protected:
    void initStyleOption(QStyleOptionViewItem *option,
                         const QModelIndex &index) const override;

private:
    QTreeView *m_parent;
    TocModel *m_model;
    QSortFilterProxyModel *m_sortModel;
}; // class WordWrapItemDelegate

} /* namespace MdEditor */
