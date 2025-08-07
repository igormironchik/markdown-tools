/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "toc.h"

// shared include.
#include "utils.h"

namespace MdEditor
{

StringData::StringData(const QString &t,
                       bool c,
                       bool rtl)
    : m_data({t,
              c,
              rtl})
    , m_splittedText(splitString(m_data.m_text,
                                 true))
{
    if (m_data.m_isRightToLeft) {
        orderWords(m_splittedText);
    }
}

//
// TocData
//

TocData::TocData(const StringDataVec &t,
                 long long int l,
                 int v,
                 const QString &id,
                 TocData *p)
    : m_text(t)
    , m_id(id)
    , m_line(l)
    , m_level(v)
    , m_parent(p)
{
}

QString TocData::concatenatedText() const
{
    QString tmp;
    bool first = true;

    for (const auto &t : std::as_const(m_text)) {
        if (!first) {
            tmp.append(QStringLiteral(" "));
        }

        tmp.append(t.m_data.m_text);

        first = false;
    }

    return tmp;
}

//
// TocModelPrivate
//

struct TocModelPrivate {
    TocModelPrivate(TocModel *parent)
        : m_q(parent)
    {
    }

    QString labelToId(const QString &label) const
    {
        auto id = label;

        if (id.startsWith(QStringLiteral("#"))) {
            id.remove(0, 1);
        }

        return id;
    }

    //! Parent.
    TocModel *m_q;
    //! Model's m_data.
    std::vector<std::shared_ptr<TocData>> m_data;
}; // struct TocModelPrivate

//
// TocModel
//

TocModel::TocModel(QObject *parent)
    : QAbstractItemModel(parent)
    , m_d(new TocModelPrivate(this))
{
}

TocModel::~TocModel()
{
}

void TocModel::addTopLevelItem(const StringDataVec &text,
                               long long int line,
                               int level,
                               const QString &label)
{
    beginInsertRows(QModelIndex(), m_d->m_data.size(), m_d->m_data.size());
    m_d->m_data.push_back(std::make_shared<TocData>(text, line, level, m_d->labelToId(label)));
    endInsertRows();
}

void TocModel::addChildItem(const QModelIndex &parent,
                            const StringDataVec &text,
                            long long int line,
                            int level,
                            const QString &label)
{
    auto data = static_cast<TocData *>(parent.internalPointer());

    beginInsertRows(parent, data->m_children.size(), data->m_children.size());
    data->m_children.push_back(std::make_shared<TocData>(text, line, level, m_d->labelToId(label), data));
    endInsertRows();
}

void TocModel::clear()
{
    beginResetModel();
    m_d->m_data.clear();
    endResetModel();
}

int TocModel::level(const QModelIndex &index) const
{
    return static_cast<TocData *>(index.internalPointer())->m_level;
}

int TocModel::lineNumber(const QModelIndex &index) const
{
    return static_cast<TocData *>(index.internalPointer())->m_line;
}

StringDataVec &TocModel::stringData(const QModelIndex &index) const
{
    return static_cast<TocData *>(index.internalPointer())->m_text;
}

int TocModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return m_d->m_data.size();
    } else {
        return static_cast<TocData *>(parent.internalPointer())->m_children.size();
    }
}

int TocModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return 1;
}

QVariant TocModel::data(const QModelIndex &index,
                        int role) const
{
    if (role == Qt::DisplayRole) {
        return static_cast<TocData *>(index.internalPointer())->concatenatedText();
    } else {
        return QVariant();
    }
}

bool TocModel::setData(const QModelIndex &index,
                       const QVariant &value,
                       int role)
{
    Q_UNUSED(index)
    Q_UNUSED(value)
    Q_UNUSED(role)

    return true;
}

Qt::ItemFlags TocModel::flags(const QModelIndex &index) const
{
    return (Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

QVariant TocModel::headerData(int section,
                              Qt::Orientation orientation,
                              int role) const
{
    Q_UNUSED(section)
    Q_UNUSED(orientation)
    Q_UNUSED(role)

    return {};
}

QModelIndex TocModel::index(int row,
                            int column,
                            const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return createIndex(row, column, m_d->m_data[row].get());
    } else {
        auto data = static_cast<TocData *>(parent.internalPointer());

        return createIndex(row, column, data->m_children[row].get());
    }
}

QModelIndex TocModel::parent(const QModelIndex &index) const
{
    auto data = static_cast<TocData *>(index.internalPointer());

    if (data->m_parent) {
        int row = -1;

        if (data->m_parent->m_parent) {
            row = std::distance(data->m_parent->m_parent->m_children.cbegin(),
                                std::find_if(data->m_parent->m_parent->m_children.cbegin(),
                                             data->m_parent->m_parent->m_children.cend(),
                                             [data](const auto &dd) {
                                                 return (data->m_parent->m_line == dd->m_line);
                                             }));
        } else {
            row = std::distance(m_d->m_data.cbegin(),
                                std::find_if(m_d->m_data.cbegin(), m_d->m_data.cend(), [data](const auto &dd) {
                                    return (data->m_parent->m_line == dd->m_line);
                                }));
        }

        return createIndex(row, 0, data->m_parent);
    } else {
        return QModelIndex();
    }
}

} /* namespace MdEditor */
