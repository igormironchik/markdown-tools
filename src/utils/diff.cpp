/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-utils include.
#include "diff.h"

namespace MdUtils
{

//
// BlockLines
//

BlockLines::BlockLines(qsizetype start,
                       qsizetype end,
                       BlockLines *parent)
    : m_start(start)
    , m_end(end)
    , m_parent(parent)
{
}

bool BlockLines::find(qsizetype line,
                      QVector<BlockLines *> *ret) const
{
    const auto it = std::lower_bound(m_children.cbegin(),
                                     m_children.cend(),
                                     line,
                                     [](const QSharedPointer<BlockLines> &block, qsizetype line) {
                                         return (line > block->m_end);
                                     });

    if (it != m_children.cend() && line >= (*it)->m_start && line <= (*it)->m_end) {
        ret->append(it->get());

        (*it)->find(line, ret);

        return true;
    }

    return false;
}

bool BlockLines::isNull() const
{
    return m_start == -1 || m_end == -1;
}

BlockLinesDiff compareBlocks(const BlockLines &oldBlock,
                             const BlockLines &newBlock)
{
    const qsizetype oldLength = oldBlock.m_end - oldBlock.m_start;
    const qsizetype newLength = newBlock.m_end - newBlock.m_start;

    if (oldLength != newLength) {
        return {newBlock.m_start, newBlock.m_end};
    }

    if (oldBlock.m_children.size() != newBlock.m_children.size()) {
        return {newBlock.m_start, newBlock.m_end};
    }

    if (newBlock.m_children.isEmpty()) {
        return {};
    }

    return compareBlocks(oldBlock.m_children, newBlock.m_children);
}

BlockLinesDiff compareBlocks(const QVector<QSharedPointer<BlockLines>> &oldBlocks,
                             const QVector<QSharedPointer<BlockLines>> &newBlocks)
{
    if (oldBlocks.isEmpty() || newBlocks.isEmpty()) {
        return {};
    }

    qsizetype oldStart = 0;
    qsizetype newStart = 0;

    while (oldStart < oldBlocks.size() && newStart < newBlocks.size()) {
        BlockLinesDiff diff = compareBlocks(*oldBlocks[oldStart], *newBlocks[newStart]);
        if (diff.m_start != -1) {
            break;
        }
        ++oldStart;
        ++newStart;
    }

    if (oldStart == oldBlocks.size() && newStart == newBlocks.size()) {
        return {};
    }

    qsizetype oldEnd = oldBlocks.size() - 1;
    qsizetype newEnd = newBlocks.size() - 1;

    while (oldEnd >= oldStart && newEnd >= newStart) {
        BlockLinesDiff diff = compareBlocks(*oldBlocks[oldEnd], *newBlocks[newEnd]);
        if (diff.m_start != -1) {
            break;
        }
        --oldEnd;
        --newEnd;
    }

    if (newEnd < newStart || oldEnd < newEnd) {
        return {};
    }

    return {newBlocks[newStart]->m_start, newBlocks[newEnd]->m_end};
}

} /* namespace MdUtils */
