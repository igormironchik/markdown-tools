/*
    SPDX-FileCopyrightText: 2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// md4qt include.
#define MD4QT_QT_SUPPORT
#include <md4qt/parser.h>
#include <md4qt/poscache.h>
#include <md4qt/traits.h>

// shared include.
#include "syntax.h"

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

//! Markdown syntax highlighter, including spelling check.
class SyntaxVisitor : public MD::PosCache<MD::QStringTrait>
{
public:
    SyntaxVisitor();
    explicit SyntaxVisitor(std::shared_ptr<MdShared::Syntax> syntax);
    ~SyntaxVisitor() override;

    SyntaxVisitor(const SyntaxVisitor &other);
    SyntaxVisitor &operator=(const SyntaxVisitor &other);

    //! Hyghlight syntax.
    void highlight(MD::StringListStream<MD::QStringTrait> *stream,
                   std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                   const Colors &cols);

    //! \return Colors scheme.
    const Colors &colors() const;
    //! Set colors scheme.
    void setColors(const Colors &c);

    //! Set font.
    void setFont(const QFont &f);

    //! Clear highlghting.
    void clearHighlighting(QTextDocument *doc);
    //! Apply highlighting.
    void applyFormats(QTextDocument *doc);

    //! \return Is spelling check enabled?
    bool isSpellingEnabled() const;
    //! Enable/disable spelling check.
    void spellingSettingsChanged(bool enabled);
    //! \return Is a given word misspelled?
    bool isMisspelled(long long int line,
                      long long int pos,
                      QPair<long long int,
                            long long int> &wordPos) const;
    //! \return Spell suggestions for word.
    QStringList spellSuggestions(const QString &word) const;
    //! \return Has this document misspelled words?
    bool hasMisspelled() const;
    //! Select next misspelled word.
    void highlightNextMisspelled(QPlainTextEdit *editor);
    //! \return Code blocks syntax highlighter.
    std::shared_ptr<MdShared::Syntax> codeBlockSyntaxHighlighter();

    //! Rectangle of highlighted code block.
    struct CodeRect {
        long long int m_startColumn = -1;
        long long int m_spacesBefore = -1;
        long long int m_startColumnLine = -1;
        long long int m_startLine = -1;
        long long int m_endLine = -1;
    }; // struct CodeRect

    //! \return Rectangles of highlighted code blocks.
    const QVector<CodeRect> &highlightedCodeRects() const;

protected:
    void onUserDefined(MD::Item<MD::QStringTrait> *i) override;
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
    void onListItem(MD::ListItem<MD::QStringTrait> *l,
                    bool first,
                    bool skipOpeningWrap = false) override;

private:
    void onItemWithOpts(MD::ItemWithOpts<MD::QStringTrait> *i);

private:
    QScopedPointer<SyntaxVisitorPrivate> m_d;
}; // class SyntaxVisitor

} /* namespace MdEditor */
