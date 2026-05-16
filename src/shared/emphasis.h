/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// md4qt include.
#include <md4qt/src/emphasis_parser.h>

namespace MdShared
{

class BaseEmphasisParser : public MD::EmphasisParser
{
public:
    explicit BaseEmphasisParser(const QChar &ch);
    ~BaseEmphasisParser() override = default;

    const QChar &symbol() const override;
    bool isEmphasis(int length) const override;
    bool isLengthCorrespond() const override;

    MD::ItemWithOpts::Styles makeStyles(int style,
                                        qsizetype startPos,
                                        qsizetype lineNumber,
                                        qsizetype length) const;

private:
    const QChar m_symbol;
}; // class BaseEmphasisParser

class SubEmphasisParser : public BaseEmphasisParser
{
public:
    explicit SubEmphasisParser(const QChar &ch);
    ~SubEmphasisParser() override = default;

    MD::ItemWithOpts::Styles openStyles(qsizetype startPos,
                                        qsizetype lineNumber,
                                        qsizetype length) const override;
    MD::ItemWithOpts::Styles closeStyles(qsizetype startPos,
                                         qsizetype lineNumber,
                                         qsizetype length) const override;
}; // class SubEmphasisParser

class SupEmphasisParser : public BaseEmphasisParser
{
public:
    explicit SupEmphasisParser(const QChar &ch);
    ~SupEmphasisParser() override = default;

    MD::ItemWithOpts::Styles openStyles(qsizetype startPos,
                                        qsizetype lineNumber,
                                        qsizetype length) const override;
    MD::ItemWithOpts::Styles closeStyles(qsizetype startPos,
                                         qsizetype lineNumber,
                                         qsizetype length) const override;
}; // class SupEmphasisParser

class HighlightEmphasisParser : public BaseEmphasisParser
{
public:
    explicit HighlightEmphasisParser(const QChar &ch);
    ~HighlightEmphasisParser() override = default;

    MD::ItemWithOpts::Styles openStyles(qsizetype startPos,
                                        qsizetype lineNumber,
                                        qsizetype length) const override;
    MD::ItemWithOpts::Styles closeStyles(qsizetype startPos,
                                         qsizetype lineNumber,
                                         qsizetype length) const override;
}; // class HighlightEmphasisParser

} /* namespace MdShared */
