/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// md4qt include.
#define MD4QT_QT_SUPPORT
#include <md4qt/poscache.h>
#include <md4qt/traits.h>
#include <md4qt/parser.h>

// Qt include.
#include <QScopedPointer>

// C++ include.
#include <vector>

QT_BEGIN_NAMESPACE
class QTextDocument;
class QPlainTextEdit;
QT_END_NAMESPACE

namespace MdEditor
{

class Editor;
struct Colors;

//
// SyntaxVisitor
//

struct SyntaxVisitorPrivate;

//! Markdown syntax highlighter.
class SyntaxVisitor : public MD::PosCache<MD::QStringTrait>
{
public:
    SyntaxVisitor();
    ~SyntaxVisitor() override;

    SyntaxVisitor(const SyntaxVisitor &other);
    SyntaxVisitor &operator=(const SyntaxVisitor &other);

    void highlight(MD::StringListStream<MD::QStringTrait> *stream,
                   std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                   const Colors &cols);

    const Colors &colors() const;
    void setColors(const Colors &c);

    void setFont(const QFont &f);

    void clearHighlighting(QTextDocument *doc);
    void applyFormats(QTextDocument *doc);

    bool isSpellingEnabled() const;
    void spellingSettingsChanged(bool enabled);
    bool isMisspelled(long long int line, long long int pos,
                      QPair<long long int, long long int> &wordPos) const;
    QStringList spellSuggestions(const QString &word) const;
    bool hasMisspelled() const;
    void highlightNextMisspelled(QPlainTextEdit *editor);

protected:
    void onReferenceLink(MD::Link<MD::QStringTrait> *l) override;
    void onText(MD::Text<MD::QStringTrait> *t) override;
    void onMath(MD::Math<MD::QStringTrait> *m) override;
    void onHeading(MD::Heading<MD::QStringTrait> *h) override;
    void onCode(MD::Code<MD::QStringTrait> *c) override;
    void onInlineCode(MD::Code<MD::QStringTrait> *c) override;
    void onBlockquote(MD::Blockquote<MD::QStringTrait> *b) override;
    void onTable(MD::Table<MD::QStringTrait> *t) override;
    void onRawHtml(MD::RawHtml<MD::QStringTrait> *h) override;
    void onHorizontalLine(MD::HorizontalLine<MD::QStringTrait> *l) override;
    void onLink(MD::Link<MD::QStringTrait> *l) override;
    void onImage(MD::Image<MD::QStringTrait> *i) override;
    void onFootnoteRef(MD::FootnoteRef<MD::QStringTrait> *ref) override;
    void onFootnote(MD::Footnote<MD::QStringTrait> *f) override;
    void onListItem(MD::ListItem<MD::QStringTrait> *l, bool first, bool skipOpeningWrap = false) override;

private:
    void onItemWithOpts(MD::ItemWithOpts<MD::QStringTrait> *i);

private:
    QScopedPointer<SyntaxVisitorPrivate> m_d;
}; // class SyntaxVisitor

} /* namespace MdEditor */
