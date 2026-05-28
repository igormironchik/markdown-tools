/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// md4qt include.
#include <md4qt/src/html.h>

namespace MdEditor
{

//
// HtmlConv
//

//! Converter to HTML.
class HtmlConv : public MD::details::HtmlVisitor
{
protected:
    QString getId(MD::Item *item) const override;
    void openStyle(const typename MD::ItemWithOpts::Styles &styles) override;
    void closeStyle(const typename MD::ItemWithOpts::Styles &styles) override;
    void onAddLineEnding() override;
    void onParagraph(MD::Paragraph *p,
                     bool wrap,
                     bool skipOpeningWrap = false) override;
    void onListItem(MD::ListItem *i,
                    bool first,
                    bool skipOpeningWrap = false) override;
    void onHeading(MD::Heading *h) override;
    void onCode(MD::Code *c) override;
    void onBlockquote(MD::Blockquote *b) override;
    void onList(MD::List *l) override;
    void onTable(MD::Table *t) override;
    void onHorizontalLine(MD::HorizontalLine *l) override;
    void onUserDefined(MD::Item *item) override;
    QString prepareTextForHtml(const QString &t) override;

private:
    void printLineId(MD::Item *item);
    void clearTop(MD::Item *item);

private:
    mutable QString m_id;
    mutable qsizetype m_count = -1;
    mutable MD::Item *m_top = nullptr;
    mutable qsizetype m_lastCount = 0;
    bool m_skipSpan = false;
}; // class HtmlConv

} /* namespace MdEditor */
