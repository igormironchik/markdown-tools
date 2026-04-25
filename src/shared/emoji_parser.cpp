/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-shared include.
#include "emoji_parser.h"
#include "emoji.h"

// md4qt include.
#include <md4qt/src/inline_context.h>
#include <md4qt/src/reverse_solidus.h>
#include <md4qt/src/text_stream.h>

// C++ include.
#include <algorithm>

namespace MdShared
{

//
// EmojiItem
//

MD::ItemType EmojiItem::type() const
{
    return emojiType();
}

QSharedPointer<MD::Item> EmojiItem::clone(MD::Document *doc) const
{
    auto d = QSharedPointer<EmojiItem>::create();
    d->applyItemWithOpts(*this);
    d->setEmojiName(emojiName());

    return d;
}

const QString &EmojiItem::emojiName() const
{
    return m_emojiName;
}

void EmojiItem::setEmojiName(const QString &name)
{
    m_emojiName = name;
}

//
// EmojiParser
//

static const QChar s_colon = QLatin1Char(':');

bool EmojiParser::check(MD::Line &line,
                        MD::ParagraphStream &,
                        MD::InlineContext &ctx,
                        QSharedPointer<MD::Document>,
                        const QString &,
                        const QString &,
                        QStringList &,
                        MD::Parser &,
                        const MD::ReverseSolidusHandler &rs)
{
    if (!rs.isPrevReverseSolidus() && line.currentChar() == s_colon) {
        const auto st = line.currentState();
        const auto startPos = line.position();
        qsizetype endPos = -1;
        QString emoji;
        bool finished = false;

        while (line.position() < line.length()) {
            line.nextChar();

            if (line.currentChar() != s_colon) {
                emoji.append(line.currentChar());
            } else {
                finished = true;
                endPos = line.position();
                line.nextChar();
                break;
            }
        }

        if (!emoji.isEmpty() && finished) {
            const auto it = std::lower_bound(s_emojiKeys.cbegin(), s_emojiKeys.cend(), emoji);

            if (it != s_emojiKeys.cend() && *it == emoji) {
                auto item = QSharedPointer<EmojiItem>::create();
                item->setStartColumn(startPos);
                item->setStartLine(line.lineNumber());
                item->setEndColumn(endPos);
                item->setEndLine(line.lineNumber());
                item->setEmojiName(emoji);

                ctx.inlines().append(item);

                return true;
            }
        }

        line.restoreState(&st);
    }

    return false;
}

QString EmojiParser::startDelimiterSymbols() const
{
    return s_colon;
}

} /* namespace MdShared */
