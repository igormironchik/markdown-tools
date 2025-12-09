/*
    SPDX-FileCopyrightText: 2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// Widgets include.
#include "emphasis.h"

namespace MdShared
{

//
// SubEmphasisParser
//

SubEmphasisParser::SubEmphasisParser(const QChar &ch)
    : m_symbol(ch)
{
}

const QChar &SubEmphasisParser::symbol() const
{
    return m_symbol;
}

bool SubEmphasisParser::isEmphasis(int) const
{
    return true;
}

bool SubEmphasisParser::isLengthCorrespond() const
{
    return false;
}

MD::ItemWithOpts::Styles SubEmphasisParser::openStyles(qsizetype startPos,
                                                       qsizetype lineNumber,
                                                       qsizetype length) const
{
    MD::ItemWithOpts::Styles styles;

    for (auto i = 0; i < length; ++i) {
        styles.append(MD::StyleDelim(16, startPos, lineNumber, startPos, lineNumber));
        ++startPos;
    }

    return styles;
}

MD::ItemWithOpts::Styles SubEmphasisParser::closeStyles(qsizetype startPos,
                                                        qsizetype lineNumber,
                                                        qsizetype length) const
{
    return openStyles(startPos, lineNumber, length);
}

//
// SupEmphasisParser
//

SupEmphasisParser::SupEmphasisParser(const QChar &ch)
    : m_symbol(ch)
{
}

const QChar &SupEmphasisParser::symbol() const
{
    return m_symbol;
}

bool SupEmphasisParser::isEmphasis(int) const
{
    return true;
}

bool SupEmphasisParser::isLengthCorrespond() const
{
    return false;
}

MD::ItemWithOpts::Styles SupEmphasisParser::openStyles(qsizetype startPos,
                                                       qsizetype lineNumber,
                                                       qsizetype length) const
{
    MD::ItemWithOpts::Styles styles;

    for (auto i = 0; i < length; ++i) {
        styles.append(MD::StyleDelim(8, startPos, lineNumber, startPos, lineNumber));
        ++startPos;
    }

    return styles;
}

MD::ItemWithOpts::Styles SupEmphasisParser::closeStyles(qsizetype startPos,
                                                        qsizetype lineNumber,
                                                        qsizetype length) const
{
    return openStyles(startPos, lineNumber, length);
}

//
// HighlightEmphasisParser
//

HighlightEmphasisParser::HighlightEmphasisParser(const QChar &ch)
    : m_symbol(ch)
{
}

const QChar &HighlightEmphasisParser::symbol() const
{
    return m_symbol;
}

bool HighlightEmphasisParser::isEmphasis(int) const
{
    return true;
}

bool HighlightEmphasisParser::isLengthCorrespond() const
{
    return false;
}

MD::ItemWithOpts::Styles HighlightEmphasisParser::openStyles(qsizetype startPos,
                                                             qsizetype lineNumber,
                                                             qsizetype length) const
{
    MD::ItemWithOpts::Styles styles;

    for (auto i = 0; i < length; ++i) {
        styles.append(MD::StyleDelim(32, startPos, lineNumber, startPos, lineNumber));
        ++startPos;
    }

    return styles;
}

MD::ItemWithOpts::Styles HighlightEmphasisParser::closeStyles(qsizetype startPos,
                                                              qsizetype lineNumber,
                                                              qsizetype length) const
{
    return openStyles(startPos, lineNumber, length);
}

} /* namespace MdShared */
