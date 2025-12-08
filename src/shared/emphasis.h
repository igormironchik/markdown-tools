/*
    SPDX-FileCopyrightText: 2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// md4qt include.
#include <md4qt/src/emphasis_parser.h>

namespace MdShared
{

class SubEmphasisParser : public MD::EmphasisParser
{
public:
    explicit SubEmphasisParser(const QChar &ch);
    ~SubEmphasisParser() override = default;

    const QChar &symbol() const override;
    bool isEmphasis(int length) const override;
    bool isLengthCorrespond() const override;
    MD::ItemWithOpts::Styles openStyles(qsizetype startPos,
                                        qsizetype lineNumber,
                                        qsizetype length) const override;
    MD::ItemWithOpts::Styles closeStyles(qsizetype startPos,
                                         qsizetype lineNumber,
                                         qsizetype length) const override;

private:
    const QChar m_symbol;
}; // class SubEmphasisParser

class SupEmphasisParser : public MD::EmphasisParser
{
public:
    explicit SupEmphasisParser(const QChar &ch);
    ~SupEmphasisParser() override = default;

    const QChar &symbol() const override;
    bool isEmphasis(int length) const override;
    bool isLengthCorrespond() const override;
    MD::ItemWithOpts::Styles openStyles(qsizetype startPos,
                                        qsizetype lineNumber,
                                        qsizetype length) const override;
    MD::ItemWithOpts::Styles closeStyles(qsizetype startPos,
                                         qsizetype lineNumber,
                                         qsizetype length) const override;

private:
    const QChar m_symbol;
}; // class SupEmphasisParser

class HighlightEmphasisParser : public MD::EmphasisParser
{
public:
    explicit HighlightEmphasisParser(const QChar &ch);
    ~HighlightEmphasisParser() override = default;

    const QChar &symbol() const override;
    bool isEmphasis(int length) const override;
    bool isLengthCorrespond() const override;
    MD::ItemWithOpts::Styles openStyles(qsizetype startPos,
                                        qsizetype lineNumber,
                                        qsizetype length) const override;
    MD::ItemWithOpts::Styles closeStyles(qsizetype startPos,
                                         qsizetype lineNumber,
                                         qsizetype length) const override;

private:
    const QChar m_symbol;
}; // class HighlightEmphasisParser

} /* namespace MdShared */
