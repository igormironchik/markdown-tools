/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "syntaxvisitor.h"
#include "colorsdlg.h"
#include "editor.h"
#include "speller.h"

// shared include.
#include "syntax.h"

// Sonnet include.
#include <Sonnet/Settings>

// Qt include.
#include <QHash>
#include <QMutexLocker>
#include <QScopedValueRollback>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextEdit>

// C++ include.
#include <algorithm>

// md4qt include.
#include <md4qt/plugins.h>

namespace MdEditor
{

//
// SyntaxVisitorPrivate
//

struct SyntaxVisitorPrivate {
    SyntaxVisitorPrivate(std::shared_ptr<MdShared::Syntax> syntax)
        : m_codeSyntax(new MdShared::Syntax)
    {
        if (syntax) {
            m_codeSyntax->setTheme(syntax->theme());
        }
    }

    SyntaxVisitorPrivate &operator=(const SyntaxVisitorPrivate &other)
    {
        if (this != &other) {
            m_colors = other.m_colors;
            m_formats = other.m_formats;
            m_font = other.m_font;
            m_stream = other.m_stream;
            m_additionalStyle = other.m_additionalStyle;
            m_spellingEnabled = other.m_spellingEnabled;
            m_skipAllUppercase = other.m_skipAllUppercase;
            m_autodetectLanguage = other.m_autodetectLanguage;
            m_skipRunTogether = other.m_skipRunTogether;
            m_ignoredWords = other.m_ignoredWords;
            m_defaultLanguage = other.m_defaultLanguage;
            m_preferredLanguages = other.m_preferredLanguages;
            m_misspelledPos = other.m_misspelledPos;
            m_currentHighlightedMisspelled = other.m_currentHighlightedMisspelled;
            m_correctWords = other.m_correctWords;
            m_codeSyntax = std::make_shared<MdShared::Syntax>();
            m_codeSyntax->setTheme(other.m_codeSyntax->theme());
            m_codeRects = other.m_codeRects;
        }

        return *this;
    }

    void clearFormats(QTextDocument *doc)
    {
        auto b = doc->firstBlock();

        while (b.isValid()) {
            b.layout()->clearFormats();

            b = b.next();
        }
    }

    void applyFormats(QTextDocument *doc)
    {
        for (auto it = m_formats.cbegin(), last = m_formats.cend(); it != last; ++it) {
            doc->findBlockByNumber(it.key()).layout()->setFormats(it.value().m_format);
        }

        m_formats.clear();
    }

    void setFormat(const QTextCharFormat &format,
                   const MD::WithPosition &pos)
    {
        setFormat(format, pos.startLine(), pos.startColumn(), pos.endLine(), pos.endColumn());
    }

    void setFormat(const QTextCharFormat &format,
                   long long int startLine,
                   long long int startColumn,
                   long long int endLine,
                   long long int endColumn)
    {
        for (auto i = startLine; i <= endLine; ++i) {
            if (m_stream) {
                const auto block = m_stream->lineAt(i);

                QTextLayout::FormatRange r;
                r.format = format;
                r.start = (i == startLine ? startColumn : 0);
                r.length = (i == startLine ? (i == endLine ? endColumn - startColumn + 1 : block.length() - startColumn)
                                           : (i == endLine ? endColumn + 1 : block.length()));

                m_formats[i].m_format.push_back(r);
            }
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

    void identifyLanguage(const QString &word)
    {
        if (m_autodetectLanguage) {
            auto &speller = Speller::instance();
            QMutexLocker lock(&speller.m_mutex);

            bool languageDetected = false;

            for (const auto &lang : std::as_const(m_preferredLanguages)) {
                speller.m_speller->setLanguage(lang);

                if (!speller.m_speller->isMisspelled(word)) {
                    languageDetected = true;
                    break;
                }
            }

            if (!languageDetected) {
                speller.m_speller->setLanguage(m_defaultLanguage);
            }
        }
    }

    //! Colors.
    Colors m_colors;

    struct Format {
        QList<QTextLayout::FormatRange> m_format;
    };

    //! Formats.
    QMap<int, Format> m_formats;
    //! Default font.
    QFont m_font;
    //! Lines stream.
    MD::StringListStream<MD::QStringTrait> *m_stream = nullptr;
    //! Additional style that should be applied for any item.
    int m_additionalStyle = 0;
    //! Is spelling check enabled?
    bool m_spellingEnabled = false;
    //! Should all uppercase words be skipped by spelling check?
    bool m_skipAllUppercase = false;
    //! Should language be autodetected by spelling check?
    bool m_autodetectLanguage = false;
    //! Should compund words be skipped by spelling check?
    bool m_skipRunTogether = false;
    //! Ignore list for spelling check.
    QStringList m_ignoredWords;
    //! Default language for spelling check.
    QString m_defaultLanguage;
    //! Preferred languages.
    QStringList m_preferredLanguages;
    //! Misspelled positions.
    QMap<long long int, QVector<QPair<long long int, long long int>>> m_misspelledPos;
    //! Current highlighted misspelled.
    QPair<long long int, long long int> m_currentHighlightedMisspelled = {-1, -1};
    //! Cache of correct words.
    QSet<QString> m_correctWords;
    //! Code syntax highlighter.
    std::shared_ptr<MdShared::Syntax> m_codeSyntax;
    //! Rectangles of code blocks that were highlighted.
    QVector<SyntaxVisitor::CodeRect> m_codeRects;
}; // struct SyntaxVisitorPrivate

//
// SyntaxVisitor
//

SyntaxVisitor::SyntaxVisitor()
    : m_d(new SyntaxVisitorPrivate(nullptr))
{
}

SyntaxVisitor::SyntaxVisitor(std::shared_ptr<MdShared::Syntax> syntax)
    : m_d(new SyntaxVisitorPrivate(syntax))
{
}

SyntaxVisitor::~SyntaxVisitor() = default;

SyntaxVisitor::SyntaxVisitor(const SyntaxVisitor &other)
    : m_d(new SyntaxVisitorPrivate(nullptr))
{
    *this = other;
}

SyntaxVisitor &SyntaxVisitor::operator=(const SyntaxVisitor &other)
{
    if (this != &other) {
        m_cache = other.m_cache;
        m_skipInCache = other.m_skipInCache;
        m_anchors = other.m_anchors;
        m_doc = other.m_doc;
        *m_d = *other.m_d;
    }

    return *this;
}

void SyntaxVisitor::setFont(const QFont &f)
{
    m_d->m_font = f;
}

void SyntaxVisitor::clearHighlighting(QTextDocument *doc)
{
    m_d->clearFormats(doc);
    m_d->m_misspelledPos.clear();
    m_d->m_currentHighlightedMisspelled = {-1, -1};
}

void SyntaxVisitor::applyFormats(QTextDocument *doc)
{
    m_d->clearFormats(doc);
    m_d->applyFormats(doc);
}

bool SyntaxVisitor::isSpellingEnabled() const
{
    return m_d->m_spellingEnabled;
}

void SyntaxVisitor::spellingSettingsChanged(bool enabled)
{
    m_d->m_spellingEnabled = enabled;

    Sonnet::Settings sonnet;
    m_d->m_skipAllUppercase = sonnet.skipUppercase();
    m_d->m_autodetectLanguage = sonnet.autodetectLanguage();
    m_d->m_skipRunTogether = sonnet.skipRunTogether();
    m_d->m_ignoredWords = sonnet.currentIgnoreList();
    m_d->m_defaultLanguage = sonnet.defaultLanguage();
    m_d->m_preferredLanguages = sonnet.preferredLanguages();

    {
        auto &speller = Speller::instance();
        QMutexLocker lock(&speller.m_mutex);
        speller.m_speller->setLanguage(m_d->m_defaultLanguage);
    }

    m_d->m_correctWords.clear();
}

bool SyntaxVisitor::isMisspelled(long long int line,
                                 long long int pos,
                                 QPair<long long int,
                                       long long int> &wordPos) const
{
    if (m_d->m_misspelledPos.contains(line)) {
        for (const auto &p : std::as_const(m_d->m_misspelledPos[line])) {
            if (pos >= p.first && pos <= p.second) {
                wordPos = p;
                return true;
            } else if (pos < p.first) {
                return false;
            }
        }
    }

    return false;
}

QStringList SyntaxVisitor::spellSuggestions(const QString &word) const
{
    QStringList ret;

    auto &speller = Speller::instance();
    QMutexLocker lock(&speller.m_mutex);

    for (const auto &lang : std::as_const(m_d->m_preferredLanguages)) {
        speller.m_speller->setLanguage(lang);
        ret.append(speller.m_speller->suggest(word));
    }

    speller.m_speller->setLanguage(m_d->m_defaultLanguage);

    return ret;
}

bool SyntaxVisitor::hasMisspelled() const
{
    return !m_d->m_misspelledPos.isEmpty();
}

std::shared_ptr<MdShared::Syntax> SyntaxVisitor::codeBlockSyntaxHighlighter()
{
    return m_d->m_codeSyntax;
}

const QVector<SyntaxVisitor::CodeRect> &SyntaxVisitor::highlightedCodeRects() const
{
    return m_d->m_codeRects;
}

void SyntaxVisitor::highlightNextMisspelled(QPlainTextEdit *editor)
{
    if (!m_d->m_misspelledPos.isEmpty()) {
        if (m_d->m_currentHighlightedMisspelled.first == -1) {
            m_d->m_currentHighlightedMisspelled = {m_d->m_misspelledPos.firstKey(), 0};
        } else {
            if (m_d->m_currentHighlightedMisspelled.first == m_d->m_misspelledPos.lastKey()
                && m_d->m_currentHighlightedMisspelled.second == m_d->m_misspelledPos.last().size() - 1) {
                m_d->m_currentHighlightedMisspelled = {m_d->m_misspelledPos.firstKey(), 0};
            } else {
                ++m_d->m_currentHighlightedMisspelled.second;

                if (m_d->m_currentHighlightedMisspelled.second
                    >= m_d->m_misspelledPos[m_d->m_currentHighlightedMisspelled.first].size()) {
                    m_d->m_currentHighlightedMisspelled.first =
                        std::next(m_d->m_misspelledPos.find(m_d->m_currentHighlightedMisspelled.first)).key();
                    m_d->m_currentHighlightedMisspelled.second = 0;
                }
            }
        }

        const auto b = editor->document()->findBlockByNumber(m_d->m_currentHighlightedMisspelled.first);
        auto c = QTextCursor(b);
        const auto p =
            m_d->m_misspelledPos[m_d->m_currentHighlightedMisspelled.first][m_d->m_currentHighlightedMisspelled.second];
        c.setPosition(b.position() + p.first);
        c.setPosition(b.position() + p.second + 1, QTextCursor::KeepAnchor);
        editor->setTextCursor(c);
    }
}

void SyntaxVisitor::highlight(MD::StringListStream<MD::QStringTrait> *stream,
                              std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                              const Colors &cols)
{
    m_d->m_misspelledPos.clear();
    m_d->m_codeRects.clear();
    m_d->m_currentHighlightedMisspelled = {-1, -1};
    m_d->m_colors = cols;
    m_d->m_stream = stream;

    MD::PosCache<MD::QStringTrait>::initialize(doc);

    m_d->m_stream = nullptr;
}

const Colors &SyntaxVisitor::colors() const
{
    return m_d->m_colors;
}

void SyntaxVisitor::setColors(const Colors &c)
{
    m_d->m_codeSyntax->setTheme(m_d->m_codeSyntax->themeForName(c.m_codeTheme));
    m_d->m_colors = c;
}

void SyntaxVisitor::onItemWithOpts(MD::ItemWithOpts<MD::QStringTrait> *i)
{
    if (m_d->m_colors.m_enabled) {
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
}

void SyntaxVisitor::onUserDefined(MD::Item<MD::QStringTrait> *i)
{
    if (m_d->m_colors.m_enabled) {
        if (i->type() == static_cast<MD::ItemType>(static_cast<int>(MD::ItemType::UserDefined) + 1)) {
            auto yaml = static_cast<MD::YAMLHeader<MD::QStringTrait> *>(i);

            QTextCharFormat format;
            format.setForeground(m_d->m_colors.m_codeColor);
            format.setFont(m_d->styleFont(m_d->m_additionalStyle));

            m_d->setFormat(format, yaml->startLine(), yaml->startColumn(), yaml->endLine(), yaml->endColumn());

            QTextCharFormat special;
            special.setForeground(m_d->m_colors.m_specialColor);
            special.setFont(m_d->styleFont(m_d->m_additionalStyle));

            m_d->setFormat(special, yaml->startDelim());
            m_d->setFormat(special, yaml->endDelim());
        }
    }

    MD::PosCache<MD::QStringTrait>::onUserDefined(i);
}

void SyntaxVisitor::onReferenceLink(MD::Link<MD::QStringTrait> *l)
{
    if (m_d->m_colors.m_enabled) {
        QTextCharFormat format;
        format.setForeground(m_d->m_colors.m_referenceColor);
        format.setFont(m_d->styleFont(m_d->m_additionalStyle));

        m_d->setFormat(format, l->startLine(), l->startColumn(), l->endLine(), l->endColumn());
    }

    MD::PosCache<MD::QStringTrait>::onReferenceLink(l);
}

namespace /* anonymous */
{

template<class String>
QString readWord(const String &str,
                 long long int &pos,
                 long long int lastPos,
                 QVector<long long int> &puncts,
                 bool &allUpper)
{
    QString word;
    const auto startPos = pos;

    QChar c;
    allUpper = true;

    while (pos <= lastPos) {
        c = str[pos];

        if (c.isSpace()) {
            break;
        } else {
            word.append(c);

            if (c.isPunct()) {
                puncts.append(pos - startPos);
            } else if (c.isLetter()) {
                if (!c.isUpper()) {
                    allUpper = false;
                }
            }

            ++pos;
        }
    }

    --pos;

    return word;
}

} /* namespace anonymous */

void SyntaxVisitor::onText(MD::Text<MD::QStringTrait> *t)
{
    QTextCharFormat format;

    if (m_d->m_colors.m_enabled) {
        format.setForeground(m_d->m_colors.m_textColor);
        format.setFont(m_d->styleFont(t->opts() | m_d->m_additionalStyle));

        m_d->setFormat(format, t->startLine(), t->startColumn(), t->endLine(), t->endColumn());
    }

    if (m_d->m_spellingEnabled && m_d->m_stream) {
        const auto block = m_d->m_stream->lineAt(t->startLine());
        auto pos = t->startColumn();

        format.setUnderlineColor(Qt::red);
        format.setFontUnderline(true);
        format.setUnderlineStyle(QTextCharFormat::WaveUnderline);

        pos = MD::skipIf(pos, block, [](const QChar &ch) {
            return (ch.isPunct() || ch.isSpace());
        });

        while (pos <= t->endColumn()) {
            const auto startPos = pos;
            QVector<long long int> puncts;
            bool allUpper = false;
            auto word = readWord(block, pos, t->endColumn(), puncts, allUpper);

            while (true) {
                m_d->identifyLanguage(word);

                const auto correctWord = m_d->m_correctWords.contains(word);
                const auto ignoredWord = m_d->m_ignoredWords.contains(word);
                bool isMisspelled = false;

                {
                    auto &speller = Speller::instance();
                    QMutexLocker lock(&speller.m_mutex);
                    isMisspelled = speller.m_speller->isMisspelled(word);
                }

                if (!correctWord && !ignoredWord && isMisspelled) {
                    if (!word.isEmpty() && word.back().isPunct()) {
                        word.removeLast();
                        puncts.pop_back();
                        continue;
                    }

                    if (!(m_d->m_skipRunTogether && !puncts.isEmpty()) && !(m_d->m_skipAllUppercase && allUpper)) {
                        m_d->setFormat(format, t->startLine(), startPos, t->startLine(), startPos + word.length() - 1);
                        m_d->m_misspelledPos[t->startLine()].append(qMakePair(startPos, startPos + word.length() - 1));
                    }
                } else if (!correctWord && !ignoredWord) {
                    m_d->m_correctWords.insert(word);
                }

                break;
            }

            pos = MD::skipIf(++pos, block, [](const QChar &ch) {
                return (ch.isPunct() || ch.isSpace());
            });
        }
    }

    onItemWithOpts(t);

    MD::PosCache<MD::QStringTrait>::onText(t);
}

void SyntaxVisitor::onMath(MD::Math<MD::QStringTrait> *m)
{
    if (m_d->m_colors.m_enabled) {
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
    }

    onItemWithOpts(m);

    MD::PosCache<MD::QStringTrait>::onMath(m);
}

void SyntaxVisitor::onHeading(MD::Heading<MD::QStringTrait> *h)
{
    QScopedValueRollback style(m_d->m_additionalStyle, m_d->m_additionalStyle | MD::BoldText);

    MD::PosCache<MD::QStringTrait>::onHeading(h);

    if (m_d->m_colors.m_enabled) {
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
}

void SyntaxVisitor::onCode(MD::Code<MD::QStringTrait> *c)
{
    if (m_d->m_colors.m_enabled) {
        {
            QTextCharFormat format;
            format.setForeground(m_d->m_colors.m_codeColor);
            format.setFont(m_d->styleFont(m_d->m_additionalStyle));

            m_d->setFormat(format, c->startLine(), c->startColumn(), c->endLine(), c->endColumn());
        }

        if (m_d->m_colors.m_codeThemeEnabled
            && m_d->m_stream
            && !m_d->m_colors.m_codeTheme.isEmpty()
            && !c->syntax().isEmpty()) {
            m_d->m_codeSyntax->setDefinition(m_d->m_codeSyntax->definitionForName(c->syntax().toLower()));

            const auto lines = c->text().split(QLatin1Char('\n'));
            const auto colored = m_d->m_codeSyntax->prepare(lines);

            qsizetype line = -1;
            long long int startColumn = 0;

            CodeRect rect;
            rect.m_startLine = c->startLine();
            rect.m_endLine = c->endLine();

            for (auto i = 0; i < colored.size(); ++i) {
                if (line != colored[i].line) {
                    line = colored[i].line;

                    const auto block = m_d->m_stream->lineAt(c->startLine() + line);
                    auto lineWithSpaces = block;
                    MD::replaceTabs<MD::QStringTrait>(lineWithSpaces);
                    const auto codeLine = lines[line].trimmed();

                    if (!codeLine.isEmpty()) {
                        auto index = block.indexOf(codeLine);
                        auto ns = MD::skipSpaces(0, lines[line]);

                        if (index >= 0) {
                            startColumn = index - ns;

                            for (; index - 1 >= 0 && ns - 1 >= 0; --index, --ns) {
                                if (block[index - 1] != lines[line][ns - 1]) {
                                    break;
                                }
                            }

                            if (rect.m_startColumn == -1 || startColumn < (rect.m_startColumn - rect.m_spacesBefore)) {
                                rect.m_startColumn = index;
                                rect.m_spacesBefore = ns;
                                rect.m_startColumnLine = c->startLine() + line;
                            }
                        }
                    }
                }

                const auto theme = m_d->m_codeSyntax->theme();

                const auto color = colored[i].format.textColor(theme);
                const auto italic = colored[i].format.isItalic(theme);
                const auto bold = colored[i].format.isBold(theme);

                QTextCharFormat format;
                format.setForeground(color);

                QScopedValueRollback style(m_d->m_additionalStyle,
                                           m_d->m_additionalStyle
                                               | (bold ? MD::BoldText : MD::TextWithoutFormat)
                                               | (italic ? MD::ItalicText : MD::TextWithoutFormat));
                format.setFont(m_d->styleFont(m_d->m_additionalStyle));

                m_d->setFormat(format,
                               c->startLine() + line,
                               colored[i].startPos + startColumn,
                               c->startLine() + line,
                               colored[i].endPos + startColumn);
            }

            m_d->m_codeRects.push_back(rect);
        }

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
    }

    onItemWithOpts(c);

    MD::PosCache<MD::QStringTrait>::onCode(c);
}

void SyntaxVisitor::onInlineCode(MD::Code<MD::QStringTrait> *c)
{
    if (m_d->m_colors.m_enabled) {
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
    }

    onItemWithOpts(c);

    MD::PosCache<MD::QStringTrait>::onInlineCode(c);
}

void SyntaxVisitor::onBlockquote(MD::Blockquote<MD::QStringTrait> *b)
{
    MD::PosCache<MD::QStringTrait>::onBlockquote(b);

    if (m_d->m_colors.m_enabled) {
        QTextCharFormat special;
        special.setForeground(m_d->m_colors.m_specialColor);
        special.setFont(m_d->styleFont(m_d->m_additionalStyle));

        for (const auto &dd : b->delims()) {
            m_d->setFormat(special, dd);
        }
    }
}

void SyntaxVisitor::onListItem(MD::ListItem<MD::QStringTrait> *l,
                               bool first,
                               bool)
{
    MD::PosCache<MD::QStringTrait>::onListItem(l, first);

    if (m_d->m_colors.m_enabled) {
        QTextCharFormat special;
        special.setForeground(m_d->m_colors.m_specialColor);
        special.setFont(m_d->styleFont(m_d->m_additionalStyle));

        m_d->setFormat(special, l->delim());

        if (l->taskDelim().startColumn() != -1) {
            m_d->setFormat(special, l->taskDelim());
        }
    }
}

void SyntaxVisitor::onTable(MD::Table<MD::QStringTrait> *t)
{
    if (m_d->m_colors.m_enabled) {
        QTextCharFormat format;
        format.setForeground(m_d->m_colors.m_tableColor);
        format.setFont(m_d->styleFont(m_d->m_additionalStyle));

        m_d->setFormat(format, t->startLine(), t->startColumn(), t->endLine(), t->endColumn());
    }

    MD::PosCache<MD::QStringTrait>::onTable(t);
}

void SyntaxVisitor::onRawHtml(MD::RawHtml<MD::QStringTrait> *h)
{
    if (m_d->m_colors.m_enabled) {
        QTextCharFormat format;
        format.setForeground(m_d->m_colors.m_htmlColor);
        format.setFont(m_d->styleFont(m_d->m_additionalStyle));

        m_d->setFormat(format, h->startLine(), h->startColumn(), h->endLine(), h->endColumn());
    }

    onItemWithOpts(h);

    MD::PosCache<MD::QStringTrait>::onRawHtml(h);
}

void SyntaxVisitor::onHorizontalLine(MD::HorizontalLine<MD::QStringTrait> *l)
{
    if (m_d->m_colors.m_enabled) {
        QTextCharFormat special;
        special.setForeground(m_d->m_colors.m_specialColor);
        special.setFont(m_d->styleFont(m_d->m_additionalStyle));

        m_d->setFormat(special, l->startLine(), l->startColumn(), l->endLine(), l->endColumn());
    }

    MD::PosCache<MD::QStringTrait>::onHorizontalLine(l);
}

void SyntaxVisitor::onLink(MD::Link<MD::QStringTrait> *l)
{
    if (m_d->m_colors.m_enabled) {
        QTextCharFormat format;
        format.setForeground(m_d->m_colors.m_linkColor);
        format.setFont(m_d->styleFont(l->opts() | m_d->m_additionalStyle));

        m_d->setFormat(format, l->startLine(), l->startColumn(), l->endLine(), l->endColumn());
    }

    QScopedValueRollback style(m_d->m_additionalStyle, m_d->m_additionalStyle | l->opts());

    MD::PosCache<MD::QStringTrait>::onLink(l);

    onItemWithOpts(l);
}

void SyntaxVisitor::onImage(MD::Image<MD::QStringTrait> *i)
{
    if (m_d->m_colors.m_enabled) {
        QTextCharFormat format;
        format.setForeground(m_d->m_colors.m_linkColor);
        format.setFont(m_d->styleFont(m_d->m_additionalStyle));

        m_d->setFormat(format, i->startLine(), i->startColumn(), i->endLine(), i->endColumn());
    }

    MD::PosCache<MD::QStringTrait>::onImage(i);

    onItemWithOpts(i);
}

void SyntaxVisitor::onFootnoteRef(MD::FootnoteRef<MD::QStringTrait> *ref)
{
    if (m_d->m_colors.m_enabled) {
        if (m_doc->footnotesMap().find(ref->id()) != m_doc->footnotesMap().cend()) {
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
    }

    MD::PosCache<MD::QStringTrait>::onFootnoteRef(ref);

    onItemWithOpts(ref);
}

void SyntaxVisitor::onFootnote(MD::Footnote<MD::QStringTrait> *f)
{
    if (m_d->m_colors.m_enabled) {
        QTextCharFormat format;
        format.setForeground(m_d->m_colors.m_referenceColor);
        format.setFont(m_d->styleFont(m_d->m_additionalStyle));

        m_d->setFormat(format, f->startLine(), f->startColumn(), f->endLine(), f->endColumn());
    }

    MD::PosCache<MD::QStringTrait>::onFootnote(f);
}

} /* namespace MdEditor */
