/*
    SPDX-FileCopyrightText: 2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// Widgets include.
#include "emphasis.h"

namespace MdShared
{

//
// BaseEmphasisParser
//

BaseEmphasisParser::BaseEmphasisParser(const QChar &ch)
    : m_symbol(ch)
{
}

const QChar &BaseEmphasisParser::symbol() const
{
    return m_symbol;
}

bool BaseEmphasisParser::isEmphasis(int) const
{
    return true;
}

bool BaseEmphasisParser::isLengthCorrespond() const
{
    return false;
}

MD::ItemWithOpts::Styles BaseEmphasisParser::makeStyles(int style,
                                                        qsizetype startPos,
                                                        qsizetype lineNumber,
                                                        qsizetype length) const
{
    MD::ItemWithOpts::Styles styles;

    for (auto i = 0; i < length; ++i) {
        styles.append(MD::StyleDelim(style, startPos, lineNumber, startPos, lineNumber));
        ++startPos;
    }

    return styles;
}

//
// SubEmphasisParser
//

SubEmphasisParser::SubEmphasisParser(const QChar &ch)
    : BaseEmphasisParser(ch)
{
}

MD::ItemWithOpts::Styles SubEmphasisParser::openStyles(qsizetype startPos,
                                                       qsizetype lineNumber,
                                                       qsizetype length) const
{
    return makeStyles(16, startPos, lineNumber, length);
}

MD::ItemWithOpts::Styles SubEmphasisParser::closeStyles(qsizetype startPos,
                                                        qsizetype lineNumber,
                                                        qsizetype length) const
{
    return makeStyles(16, startPos, lineNumber, length);
}

//
// SupEmphasisParser
//

SupEmphasisParser::SupEmphasisParser(const QChar &ch)
    : BaseEmphasisParser(ch)
{
}

MD::ItemWithOpts::Styles SupEmphasisParser::openStyles(qsizetype startPos,
                                                       qsizetype lineNumber,
                                                       qsizetype length) const
{
    return makeStyles(8, startPos, lineNumber, length);
}

MD::ItemWithOpts::Styles SupEmphasisParser::closeStyles(qsizetype startPos,
                                                        qsizetype lineNumber,
                                                        qsizetype length) const
{
    return makeStyles(8, startPos, lineNumber, length);
}

//
// HighlightEmphasisParser
//

HighlightEmphasisParser::HighlightEmphasisParser(const QChar &ch)
    : BaseEmphasisParser(ch)
{
}

MD::ItemWithOpts::Styles HighlightEmphasisParser::openStyles(qsizetype startPos,
                                                             qsizetype lineNumber,
                                                             qsizetype length) const
{
    return makeStyles(32, startPos, lineNumber, length);
}

MD::ItemWithOpts::Styles HighlightEmphasisParser::closeStyles(qsizetype startPos,
                                                              qsizetype lineNumber,
                                                              qsizetype length) const
{
    return makeStyles(32, startPos, lineNumber, length);
}

} /* namespace MdShared */
