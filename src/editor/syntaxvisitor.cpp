/*
    SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "syntaxvisitor.hpp"
#include "colorsdlg.hpp"
#include "editor.hpp"

// KF6 include.
#include <Sonnet/Speller>

// Qt include.
#include <QScopedValueRollback>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextEdit>

// C++ include.
#include <algorithm>

namespace MdEditor
{

//
// SyntaxVisitorPrivate
//

struct SyntaxVisitorPrivate {
    SyntaxVisitorPrivate(Editor *e)
        : m_editor(e)
        , m_speller(new Sonnet::Speller)
    {
    }

    void clearFormats()
    {
        auto b = m_editor->document()->firstBlock();

        while (b.isValid()) {
            b.layout()->clearFormats();

            b = b.next();
        }

        m_formats.clear();
    }

    void applyFormats()
    {
        for (const auto &f : std::as_const(m_formats)) {
            f.m_block.layout()->setFormats(f.m_format);
        }
    }

    void setFormat(const QTextCharFormat &format, const MD::WithPosition &pos)
    {
        setFormat(format, pos.startLine(), pos.startColumn(), pos.endLine(), pos.endColumn());
    }

    void setFormat(const QTextCharFormat &format, long long int startLine, long long int startColumn, long long int endLine, long long int endColumn)
    {
        for (auto i = startLine; i <= endLine; ++i) {
            m_formats[i].m_block = m_editor->document()->findBlockByNumber(i);

            QTextLayout::FormatRange r;
            r.format = format;
            r.start = (i == startLine ? startColumn : 0);
            r.length = (i == startLine ? (i == endLine ? endColumn - startColumn + 1 : m_formats[i].m_block.length() - 1 - startColumn)
                                       : (i == endLine ? endColumn + 1 : m_formats[i].m_block.length() - 1));

            m_formats[i].m_format.push_back(r);
        }
    }

    QFont styleFont(int opts) const
    {
        auto f = m_font;

        if (opts & MD::ItalicText) {
            f.setItalic(true);
        }

        if (opts & MD::BoldText) {
            f.setBold(true);
        }

        if (opts & MD::StrikethroughText) {
            f.setStrikeOut(true);
        }

        return f;
    }

    //! Editor.
    Editor *m_editor = nullptr;
    //! Document.
    std::shared_ptr<MD::Document<MD::QStringTrait>> m_doc;
    //! Colors.
    Colors m_colors;

    struct Format {
        QTextBlock m_block;
        QList<QTextLayout::FormatRange> m_format;
    };

    //! Formats.
    QMap<int, Format> m_formats;
    //! Default m_font.
    QFont m_font;
    //! Additional style that should be applied for any item.
    int m_additionalStyle = 0;
    //! Spell checker.
    Sonnet::Speller * m_speller = nullptr;
    //! Is spelling check enabled?
    bool m_spellingEnabled = false;
}; // struct SyntaxVisitorPrivate

//
// SyntaxVisitor
//

SyntaxVisitor::SyntaxVisitor(Editor *editor)
    : m_d(new SyntaxVisitorPrivate(editor))
{
}

SyntaxVisitor::~SyntaxVisitor()
{
}

void SyntaxVisitor::setFont(const QFont &f)
{
    m_d->m_font = f;
}

void SyntaxVisitor::clearHighlighting()
{
    m_d->clearFormats();
}

void SyntaxVisitor::spellingSettingsChanged(bool enabled)
{
    m_d->m_spellingEnabled = enabled;
}

void SyntaxVisitor::highlight(std::shared_ptr<MD::Document<MD::QStringTrait>> doc, const Colors &colors)
{
    m_d->clearFormats();

    m_d->m_doc = doc;
    m_d->m_colors = colors;

    MD::PosCache<MD::QStringTrait>::initialize(m_d->m_doc);

    if (colors.m_enabled) {
        m_d->applyFormats();
    } else {
        m_d->m_formats.clear();
    }
}

void SyntaxVisitor::onItemWithOpts(MD::ItemWithOpts<MD::QStringTrait> *i)
{
    QTextCharFormat special;
    special.setForeground(m_d->m_colors.m_specialColor);
    special.setFont(m_d->styleFont(m_d->m_additionalStyle));

    for (const auto &s : i->openStyles()) {
        m_d->setFormat(special, s);
    }

    for (const auto &s : i->closeStyles()) {
        m_d->setFormat(special, s);
    }
}

void SyntaxVisitor::onReferenceLink(MD::Link<MD::QStringTrait> *l)
{
    QTextCharFormat format;
    format.setForeground(m_d->m_colors.m_referenceColor);
    format.setFont(m_d->styleFont(m_d->m_additionalStyle));

    m_d->setFormat(format, l->startLine(), l->startColumn(), l->endLine(), l->endColumn());

    MD::PosCache<MD::QStringTrait>::onReferenceLink(l);
}

namespace /* anonymous */ {

long long int
skipSpacesAndPunct(const QTextDocument *doc, long long int pos)
{
    auto c = doc->characterAt(pos);

    while (!c.isNull()) {
        if (!(c.isPunct() || c.isSpace())) {
            break;
        }

        c = doc->characterAt(++pos);
    }

    return pos;
}

void
skipLastPunct(QString &word, long long int &pos)
{
    while (!word.isEmpty() && word.back().isPunct()) {
        --pos;
        word.removeLast();
    }
}

QString
readWord(const QTextDocument *doc, long long int &pos, long long int lastPos)
{
    QString word;

    auto c = doc->characterAt(pos);

    while (!c.isNull() && pos <= lastPos) {
        if (c.isSpace()) {
            break;
        } else {
            word.append(c);
            ++pos;
            c = doc->characterAt(pos);
        }
    }

    --pos;

    skipLastPunct(word, pos);

    return word;
}

} /* namespace anonymous */

void SyntaxVisitor::onText(MD::Text<MD::QStringTrait> *t)
{
    QTextCharFormat format;
    format.setForeground(m_d->m_colors.m_textColor);
    format.setFont(m_d->styleFont(t->opts() | m_d->m_additionalStyle));

    m_d->setFormat(format, t->startLine(), t->startColumn(), t->endLine(), t->endColumn());

    if (m_d->m_spellingEnabled) {
        const auto block = m_d->m_editor->document()->findBlockByNumber(t->startLine());
        auto pos = block.position() + t->startColumn();

        format.setUnderlineColor(Qt::red);
        format.setFontUnderline(true);
        format.setUnderlineStyle(QTextCharFormat::WaveUnderline);

        pos = skipSpacesAndPunct(m_d->m_editor->document(), pos);

        while (pos - block.position() <= t->endColumn()) {
            const auto startPos = pos - block.position();
            auto word = readWord(m_d->m_editor->document(), pos, block.position() + t->endColumn());

            if (m_d->m_speller->isMisspelled(word)) {
                m_d->setFormat(format, t->startLine(), startPos, t->startLine(), startPos + word.length() - 1);
            }

            pos = skipSpacesAndPunct(m_d->m_editor->document(), ++pos);
        }
    }

    onItemWithOpts(t);

    MD::PosCache<MD::QStringTrait>::onText(t);
}

void SyntaxVisitor::onMath(MD::Math<MD::QStringTrait> *m)
{
    QTextCharFormat format;
    format.setForeground(m_d->m_colors.m_mathColor);
    format.setFont(m_d->styleFont(m_d->m_additionalStyle));

    m_d->setFormat(format, m->startLine(), m->startColumn(), m->endLine(), m->endColumn());

    QTextCharFormat special;
    special.setForeground(m_d->m_colors.m_specialColor);
    special.setFont(m_d->styleFont(m_d->m_additionalStyle));

    if (m->startDelim().startColumn() != -1) {
        m_d->setFormat(special, m->startDelim());
    }

    if (m->endDelim().startColumn() != -1) {
        m_d->setFormat(special, m->endDelim());
    }

    if (m->syntaxPos().startColumn() != -1) {
        m_d->setFormat(special, m->syntaxPos());
    }

    onItemWithOpts(m);

    MD::PosCache<MD::QStringTrait>::onMath(m);
}

void SyntaxVisitor::onHeading(MD::Heading<MD::QStringTrait> *h)
{
    QScopedValueRollback style(m_d->m_additionalStyle, m_d->m_additionalStyle | MD::BoldText);

    MD::PosCache<MD::QStringTrait>::onHeading(h);

    QTextCharFormat special;
    special.setForeground(m_d->m_colors.m_specialColor);
    special.setFont(m_d->styleFont(MD::BoldText));

    if (!h->delims().empty()) {
        for (const auto &delim : h->delims()) {
            m_d->setFormat(special, delim);
        }
    }

    if (h->labelPos().startColumn() != -1) {
        m_d->setFormat(special, h->labelPos());
    }
}

void SyntaxVisitor::onCode(MD::Code<MD::QStringTrait> *c)
{
    QTextCharFormat format;
    format.setForeground(m_d->m_colors.m_codeColor);
    format.setFont(m_d->styleFont(m_d->m_additionalStyle));

    m_d->setFormat(format, c->startLine(), c->startColumn(), c->endLine(), c->endColumn());

    QTextCharFormat special;
    special.setForeground(m_d->m_colors.m_specialColor);
    special.setFont(m_d->styleFont(m_d->m_additionalStyle));

    if (c->startDelim().startColumn() != -1) {
        m_d->setFormat(special, c->startDelim());
    }

    if (c->endDelim().startColumn() != -1) {
        m_d->setFormat(special, c->endDelim());
    }

    if (c->syntaxPos().startColumn() != -1) {
        m_d->setFormat(special, c->syntaxPos());
    }

    onItemWithOpts(c);

    MD::PosCache<MD::QStringTrait>::onCode(c);
}

void SyntaxVisitor::onInlineCode(MD::Code<MD::QStringTrait> *c)
{
    QTextCharFormat format;
    format.setForeground(m_d->m_colors.m_inlineColor);
    format.setFont(m_d->styleFont(m_d->m_additionalStyle));

    m_d->setFormat(format, c->startLine(), c->startColumn(), c->endLine(), c->endColumn());

    QTextCharFormat special;
    special.setForeground(m_d->m_colors.m_specialColor);
    special.setFont(m_d->styleFont(m_d->m_additionalStyle));

    if (c->startDelim().startColumn() != -1) {
        m_d->setFormat(special, c->startDelim());
    }

    if (c->endDelim().startColumn() != -1) {
        m_d->setFormat(special, c->endDelim());
    }

    onItemWithOpts(c);

    MD::PosCache<MD::QStringTrait>::onInlineCode(c);
}

void SyntaxVisitor::onBlockquote(MD::Blockquote<MD::QStringTrait> *b)
{
    MD::PosCache<MD::QStringTrait>::onBlockquote(b);

    QTextCharFormat special;
    special.setForeground(m_d->m_colors.m_specialColor);
    special.setFont(m_d->styleFont(m_d->m_additionalStyle));

    for (const auto &dd : b->delims()) {
        m_d->setFormat(special, dd);
    }
}

void SyntaxVisitor::onListItem(MD::ListItem<MD::QStringTrait> *l, bool first)
{
    MD::PosCache<MD::QStringTrait>::onListItem(l, first);

    QTextCharFormat special;
    special.setForeground(m_d->m_colors.m_specialColor);
    special.setFont(m_d->styleFont(m_d->m_additionalStyle));

    m_d->setFormat(special, l->delim());

    if (l->taskDelim().startColumn() != -1) {
        m_d->setFormat(special, l->taskDelim());
    }
}

void SyntaxVisitor::onTable(MD::Table<MD::QStringTrait> *t)
{
    QTextCharFormat format;
    format.setForeground(m_d->m_colors.m_tableColor);
    format.setFont(m_d->styleFont(m_d->m_additionalStyle));

    m_d->setFormat(format, t->startLine(), t->startColumn(), t->endLine(), t->endColumn());

    MD::PosCache<MD::QStringTrait>::onTable(t);
}

void SyntaxVisitor::onRawHtml(MD::RawHtml<MD::QStringTrait> *h)
{
    QTextCharFormat format;
    format.setForeground(m_d->m_colors.m_htmlColor);
    format.setFont(m_d->styleFont(m_d->m_additionalStyle));

    m_d->setFormat(format, h->startLine(), h->startColumn(), h->endLine(), h->endColumn());

    onItemWithOpts(h);

    MD::PosCache<MD::QStringTrait>::onRawHtml(h);
}

void SyntaxVisitor::onHorizontalLine(MD::HorizontalLine<MD::QStringTrait> *l)
{
    QTextCharFormat special;
    special.setForeground(m_d->m_colors.m_specialColor);
    special.setFont(m_d->styleFont(m_d->m_additionalStyle));

    m_d->setFormat(special, l->startLine(), l->startColumn(), l->endLine(), l->endColumn());

    MD::PosCache<MD::QStringTrait>::onHorizontalLine(l);
}

void SyntaxVisitor::onLink(MD::Link<MD::QStringTrait> *l)
{
    QTextCharFormat format;
    format.setForeground(m_d->m_colors.m_linkColor);
    format.setFont(m_d->styleFont(l->opts() | m_d->m_additionalStyle));

    m_d->setFormat(format, l->startLine(), l->startColumn(), l->endLine(), l->endColumn());

    QScopedValueRollback style(m_d->m_additionalStyle, m_d->m_additionalStyle | l->opts());

    MD::PosCache<MD::QStringTrait>::onLink(l);

    onItemWithOpts(l);
}

void SyntaxVisitor::onImage(MD::Image<MD::QStringTrait> *i)
{
    QTextCharFormat format;
    format.setForeground(m_d->m_colors.m_linkColor);
    format.setFont(m_d->styleFont(m_d->m_additionalStyle));

    m_d->setFormat(format, i->startLine(), i->startColumn(), i->endLine(), i->endColumn());

    MD::PosCache<MD::QStringTrait>::onImage(i);

    onItemWithOpts(i);
}

void SyntaxVisitor::onFootnoteRef(MD::FootnoteRef<MD::QStringTrait> *ref)
{
    if (m_d->m_doc->footnotesMap().find(ref->id()) != m_d->m_doc->footnotesMap().cend()) {
        QTextCharFormat format;
        format.setForeground(m_d->m_colors.m_linkColor);
        format.setFont(m_d->styleFont(ref->opts() | m_d->m_additionalStyle));

        m_d->setFormat(format, ref->startLine(), ref->startColumn(), ref->endLine(), ref->endColumn());
    } else {
        QTextCharFormat format;
        format.setForeground(m_d->m_colors.m_textColor);
        format.setFont(m_d->styleFont(ref->opts() | m_d->m_additionalStyle));

        m_d->setFormat(format, ref->startLine(), ref->startColumn(), ref->endLine(), ref->endColumn());
    }

    MD::PosCache<MD::QStringTrait>::onFootnoteRef(ref);

    onItemWithOpts(ref);
}

void SyntaxVisitor::onFootnote(MD::Footnote<MD::QStringTrait> *f)
{
    QTextCharFormat format;
    format.setForeground(m_d->m_colors.m_referenceColor);
    format.setFont(m_d->styleFont(m_d->m_additionalStyle));

    m_d->setFormat(format, f->startLine(), f->startColumn(), f->endLine(), f->endColumn());

    MD::PosCache<MD::QStringTrait>::onFootnote(f);
}

} /* namespace MdEditor */
