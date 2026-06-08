/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QSharedPointer>
#include <QVector>

namespace MdUtils
{

//
// BlockLines
//

//! Information about lines of the block.
struct BlockLines {
    BlockLines() = default;
    BlockLines(qsizetype start,
               qsizetype end,
               BlockLines *parent = nullptr);

    //! Set vector of blocks (from top most parent to most nested children) with the given line.
    bool find(qsizetype line,
              QVector<BlockLines *> *ret) const;

    //! \return Whether this block is null.
    bool isNull() const;

    qsizetype m_start = -1;
    qsizetype m_end = -1;
    BlockLines *m_parent = nullptr;
    // Should be sorted.
    QVector<QSharedPointer<BlockLines>> m_children;
}; // struct BlockLines

//
// BlockLinesDiff
//

//! Difference between two blocks of lines.
struct BlockLinesDiff {
    qsizetype m_start = -1;
    qsizetype m_end = -1;
}; // struct BlockLinesDiff

//! Compare blocks of lines This is not a strict difference, it reports the difference
//! when unfolding is needed.
BlockLinesDiff compareBlocks(const QVector<QSharedPointer<BlockLines>> &oldBlocks,
                             const QVector<QSharedPointer<BlockLines>> &newBlocks);

} /* namespace MdUtils */
